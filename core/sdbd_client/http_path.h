// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Shared fake-HTTP path normalize + handler lookup. Match policy lives
// in traits so sg and core::sdb fakes stay thin.

#pragma once

#include <string>

namespace sg {
namespace detail {

inline std::string normalize_http_path(const std::string& url) {
  std::string path = url;
  const auto scheme = path.find("://");
  if (scheme != std::string::npos) {
    const auto slash = path.find('/', scheme + 3);
    if (slash == std::string::npos) {
      return "/";
    }
    path = path.substr(slash);
  }
  const auto q = path.find('?');
  if (q != std::string::npos) {
    path.resize(q);
  }
  return path;
}

struct prefix_or_contains_match {
  static constexpr bool prefer_exact = true;

  static bool matches(const std::string& path, const std::string& key) {
    return path.rfind(key, 0) == 0 || path.find(key) != std::string::npos;
  }
};

struct suffix_or_contains_match {
  static constexpr bool prefer_exact = false;

  static bool matches(const std::string& path, const std::string& key) {
    if (path.size() >= key.size() &&
        path.compare(path.size() - key.size(), key.size(), key) == 0) {
      return true;
    }
    return path.find(key) != std::string::npos;
  }
};

template <typename Match, typename Map>
auto find_http_handler(const Map& handlers, const std::string& path) ->
    typename Map::mapped_type {
  if constexpr (Match::prefer_exact) {
    auto it = handlers.find(path);
    if (it != handlers.end()) {
      return it->second;
    }
  }
  for (const auto& kv : handlers) {
    if (Match::matches(path, kv.first)) {
      return kv.second;
    }
  }
  return {};
}

}  // namespace detail
}  // namespace sg
