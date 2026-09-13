// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: WinHTTP NO_PROXY (WSL 127.0.0.1). Body is winhttp_send<Traits>.

#include "http_transport.h"
#include "winhttp_send.h"

#if defined(_WIN32)

namespace sg {
namespace {

class WinHttpTransport final : public HttpTransport {
 public:
  HttpResponse Send(const HttpRequest& req) override {
    return winhttp_send<default_winhttp_traits, HttpRequest, HttpResponse>(req);
  }
};

}  // namespace

std::unique_ptr<HttpTransport> CreateDefaultHttpTransport() {
  return std::make_unique<WinHttpTransport>();
}

}  // namespace sg

#else

namespace sg {
namespace {

class StubHttpTransport final : public HttpTransport {
 public:
  HttpResponse Send(const HttpRequest&) override {
    HttpResponse r;
    r.error = "WinHTTP transport is Windows-only; inject FakeHttpTransport";
    return r;
  }
};

}  // namespace

std::unique_ptr<HttpTransport> CreateDefaultHttpTransport() {
  return std::make_unique<StubHttpTransport>();
}

}  // namespace sg

#endif
