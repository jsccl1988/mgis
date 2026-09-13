// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Shared WinHTTP POST/GET. User-agent and timeouts live in traits so sg
// and core::sdb transports stay one-liners.

#pragma once

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>

#include <concepts>
#include <string>
#include <utility>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace sg {
namespace detail {

inline std::wstring utf8_to_wide(const std::string& s) {
  if (s.empty()) {
    return {};
  }
  const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(),
                                    static_cast<int>(s.size()), nullptr, 0);
  std::wstring out(static_cast<std::size_t>(n), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &out[0],
                      n);
  return out;
}

struct WinHttpHandle {
  HINTERNET h = nullptr;

  WinHttpHandle() = default;
  explicit WinHttpHandle(HINTERNET handle) : h(handle) {}
  ~WinHttpHandle() { reset(); }

  WinHttpHandle(const WinHttpHandle&) = delete;
  WinHttpHandle& operator=(const WinHttpHandle&) = delete;

  WinHttpHandle(WinHttpHandle&& other) noexcept : h(other.h) {
    other.h = nullptr;
  }
  WinHttpHandle& operator=(WinHttpHandle&& other) noexcept {
    if (this != &other) {
      reset();
      h = other.h;
      other.h = nullptr;
    }
    return *this;
  }

  void reset() {
    if (h) {
      WinHttpCloseHandle(h);
      h = nullptr;
    }
  }

  explicit operator bool() const { return h != nullptr; }
};

}  // namespace detail

template <typename T>
concept winhttp_ua_traits = requires {
  { T::user_agent() } -> std::convertible_to<const wchar_t*>;
};

template <typename T>
concept winhttp_request_like = requires(const T& r) {
  r.method;
  r.url;
  r.body;
  r.headers;
};

template <typename T>
concept winhttp_response_like = std::default_initializable<T> && requires(T r) {
  r.status;
  r.body;
  r.error;
};

struct default_winhttp_traits {
  static constexpr const wchar_t* user_agent() {
    return L"mgis-SdbdClient/0.1 (pending-pin)";
  }
  static constexpr int resolve_timeout_ms = 3000;
  static constexpr int connect_timeout_ms = 3000;
  static constexpr int send_timeout_ms = 3000;
  static constexpr int receive_timeout_ms = 3000;
};

struct legacy_sdb_winhttp_traits {
  static constexpr const wchar_t* user_agent() {
    return L"MGIS-SdbdClient/0.1";
  }
  static constexpr int resolve_timeout_ms = 3000;
  static constexpr int connect_timeout_ms = 3000;
  static constexpr int send_timeout_ms = 3000;
  static constexpr int receive_timeout_ms = 3000;
};

template <winhttp_ua_traits Traits, winhttp_request_like Request,
          winhttp_response_like Response>
Response winhttp_send(const Request& req) {
  Response out;
  URL_COMPONENTS uc{};
  uc.dwStructSize = sizeof(uc);
  uc.dwSchemeLength = static_cast<DWORD>(-1);
  uc.dwHostNameLength = static_cast<DWORD>(-1);
  uc.dwUrlPathLength = static_cast<DWORD>(-1);
  uc.dwExtraInfoLength = static_cast<DWORD>(-1);

  std::wstring wurl = detail::utf8_to_wide(req.url);
  if (!WinHttpCrackUrl(wurl.c_str(), static_cast<DWORD>(wurl.size()), 0, &uc)) {
    out.error = "WinHttpCrackUrl failed";
    return out;
  }

  std::wstring host(uc.lpszHostName, uc.dwHostNameLength);
  std::wstring path(uc.lpszUrlPath, uc.dwUrlPathLength);
  if (uc.dwExtraInfoLength > 0 && uc.lpszExtraInfo) {
    path.append(uc.lpszExtraInfo, uc.dwExtraInfoLength);
  }

  detail::WinHttpHandle session(WinHttpOpen(
      Traits::user_agent(), WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME,
      WINHTTP_NO_PROXY_BYPASS, 0));
  if (!session) {
    out.error = "WinHttpOpen failed";
    return out;
  }

  // Default WinHTTP waits are 30–60s. A peer that accepts TCP but never
  // completes HTTP would freeze the GUI thread.
  int resolve_ms = 3000;
  int connect_ms = 3000;
  int send_ms = 3000;
  int receive_ms = 3000;
  if constexpr (requires { Traits::resolve_timeout_ms; }) {
    resolve_ms = Traits::resolve_timeout_ms;
    connect_ms = Traits::connect_timeout_ms;
    send_ms = Traits::send_timeout_ms;
    receive_ms = Traits::receive_timeout_ms;
  }
  WinHttpSetTimeouts(session.h, resolve_ms, connect_ms, send_ms, receive_ms);

  detail::WinHttpHandle connect(
      WinHttpConnect(session.h, host.c_str(), uc.nPort, 0));
  if (!connect) {
    out.error = "WinHttpConnect failed";
    return out;
  }

  const DWORD flags =
      (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
  std::wstring method = detail::utf8_to_wide(req.method.empty() ? "GET" : req.method);
  detail::WinHttpHandle request(WinHttpOpenRequest(
      connect.h, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, flags));
  if (!request) {
    out.error = "WinHttpOpenRequest failed";
    return out;
  }

  std::wstring headers;
  for (const auto& h : req.headers) {
    headers += detail::utf8_to_wide(h.first);
    headers += L": ";
    headers += detail::utf8_to_wide(h.second);
    headers += L"\r\n";
  }

  BOOL ok = WinHttpSendRequest(
      request.h, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
      headers.empty() ? 0 : static_cast<DWORD>(-1L),
      req.body.empty() ? WINHTTP_NO_REQUEST_DATA
                       : const_cast<char*>(req.body.data()),
      static_cast<DWORD>(req.body.size()), static_cast<DWORD>(req.body.size()),
      0);
  if (!ok) {
    out.error = "WinHttpSendRequest failed";
    return out;
  }

  if (!WinHttpReceiveResponse(request.h, nullptr)) {
    out.error = "WinHttpReceiveResponse failed";
    return out;
  }

  DWORD status = 0;
  DWORD status_size = sizeof(status);
  WinHttpQueryHeaders(request.h,
                      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                      WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                      WINHTTP_NO_HEADER_INDEX);
  out.status = static_cast<int>(status);

  std::string body;
  for (;;) {
    DWORD avail = 0;
    if (!WinHttpQueryDataAvailable(request.h, &avail)) {
      out.error = "WinHttpQueryDataAvailable failed";
      break;
    }
    if (avail == 0) {
      break;
    }
    std::vector<char> buf(avail);
    DWORD read = 0;
    if (!WinHttpReadData(request.h, buf.data(), avail, &read)) {
      out.error = "WinHttpReadData failed";
      break;
    }
    body.append(buf.data(), read);
  }
  out.body = std::move(body);
  return out;
}

}  // namespace sg

#endif  // _WIN32
