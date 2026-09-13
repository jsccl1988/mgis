// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Shared mapd/sdbd link parse. Service-specific bits live in traits so
// ParseMapdLink / ParseSdbdLink stay thin pin-stable wrappers. Optional
// default_port / extra_forbidden_ports via if constexpr requires.

#pragma once

#include <cctype>
#include <concepts>
#include <map>
#include <string>
#include <string_view>

namespace sg {

struct SpatialLink {
  std::string base_url;
  std::string layer;
  std::string error;
};

enum class SpatialService { kMapd, kSdbd };

template <SpatialService>
struct spatial_link_traits;

template <>
struct spatial_link_traits<SpatialService::kMapd> {
  static constexpr std::string_view alias_scheme = "mapd";
  static constexpr std::string_view forbidden_port = "8021";
  static constexpr std::string_view forbidden_error =
      "sdbd port :8021; use link sdbd";
};

template <>
struct spatial_link_traits<SpatialService::kSdbd> {
  static constexpr std::string_view alias_scheme = "sdbd";
  static constexpr std::string_view forbidden_port = "8020";
  static constexpr std::string_view forbidden_error =
      "mapd port :8020; use link mapd";
};

template <typename T>
concept spatial_link_like = requires {
  { T::alias_scheme } -> std::convertible_to<std::string_view>;
  { T::forbidden_port } -> std::convertible_to<std::string_view>;
  { T::forbidden_error } -> std::convertible_to<std::string_view>;
};

namespace detail {

inline std::string trim_ws(std::string s) {
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
    s.erase(s.begin());
  }
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
    s.pop_back();
  }
  return s;
}

inline std::string trim_trailing_slash(std::string s) {
  while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
    s.pop_back();
  }
  return s;
}

inline void parse_query(const std::string& q,
                        std::map<std::string, std::string>* out) {
  if (!out) {
    return;
  }
  std::size_t i = 0;
  while (i < q.size()) {
    auto amp = q.find('&', i);
    if (amp == std::string::npos) {
      amp = q.size();
    }
    const std::string pair = q.substr(i, amp - i);
    auto eq = pair.find('=');
    if (eq != std::string::npos) {
      (*out)[pair.substr(0, eq)] = pair.substr(eq + 1);
    }
    i = amp + 1;
  }
}

}  // namespace detail

template <spatial_link_like Traits>
bool parse_spatial_link(const std::string& input, SpatialLink* out) {
  if (!out) {
    return false;
  }
  *out = SpatialLink{};
  std::string s = detail::trim_ws(input);
  if (s.empty()) {
    out->error = "empty link";
    return false;
  }

  std::string scheme = "http";
  auto sep = s.find("://");
  std::string rest = s;
  if (sep != std::string::npos) {
    scheme = s.substr(0, sep);
    rest = s.substr(sep + 3);
    for (char& c : scheme) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
  }

  if (scheme == Traits::alias_scheme) {
    scheme = "http";
  } else if (scheme != "http" && scheme != "https") {
    out->error = "unsupported scheme";
    return false;
  }

  std::string hostport = rest;
  std::string path_query;
  auto slash = rest.find('/');
  if (slash != std::string::npos) {
    hostport = rest.substr(0, slash);
    path_query = rest.substr(slash);
  }

  std::string query;
  auto qpos = path_query.find('?');
  if (qpos != std::string::npos) {
    query = path_query.substr(qpos + 1);
  }

  std::string port;
  if (!hostport.empty() && hostport.front() == '[') {
    const auto rb = hostport.find(']');
    if (rb != std::string::npos && rb + 1 < hostport.size() &&
        hostport[rb + 1] == ':') {
      port = hostport.substr(rb + 2);
    }
  } else {
    const auto colon = hostport.rfind(':');
    if (colon != std::string::npos) {
      port = hostport.substr(colon + 1);
    }
  }

  if (port.empty()) {
    if constexpr (requires { Traits::default_port; }) {
      port = std::to_string(Traits::default_port);
      hostport += ':';
      hostport += port;
    }
  }

  if (port == Traits::forbidden_port) {
    out->error.assign(Traits::forbidden_error);
    return false;
  }
  if constexpr (requires { Traits::extra_forbidden_ports; }) {
    for (const auto extra : Traits::extra_forbidden_ports) {
      if (port == extra) {
        if constexpr (requires { Traits::extra_forbidden_error; }) {
          out->error.assign(Traits::extra_forbidden_error);
        } else {
          out->error = "bad_request";
        }
        return false;
      }
    }
  }

  std::map<std::string, std::string> kv;
  detail::parse_query(query, &kv);
  if (kv.count("layer")) {
    out->layer = kv["layer"];
  }

  out->base_url = detail::trim_trailing_slash(scheme + "://" + hostport);
  return !out->base_url.empty();
}

template <SpatialService S>
bool parse_spatial_link(const std::string& input, SpatialLink* out) {
  return parse_spatial_link<spatial_link_traits<S>>(input, out);
}

}  // namespace sg
