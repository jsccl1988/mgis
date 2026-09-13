// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: layout matches smartgis `include/sg/clients/http_transport.h`.

#pragma once

#include <concepts>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace sg {

struct HttpRequest {
  std::string method;  // "GET" | "POST"
  std::string url;
  std::string body;
  std::map<std::string, std::string> headers;
  enum class PathClass : std::uint8_t { kMeta = 0, kQ = 1, kR = 2 };
  PathClass path_class = PathClass::kMeta;
};

struct HttpResponse {
  int status = 0;
  std::string body;
  std::string error;
};

class HttpTransport {
 public:
  virtual ~HttpTransport() = default;
  virtual HttpResponse Send(const HttpRequest& req) = 0;
};

template <typename T>
concept http_transport_like = requires(T& t, const HttpRequest& req) {
  { t.Send(req) } -> std::same_as<HttpResponse>;
};

std::unique_ptr<HttpTransport> CreateDefaultHttpTransport();

}  // namespace sg
