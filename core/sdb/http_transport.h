// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef SDB_HTTP_TRANSPORT_H
#define SDB_HTTP_TRANSPORT_H

#include <map>
#include <memory>
#include <string>

namespace core {
namespace sdb {

struct HttpRequest {
  std::string method;
  std::string url;
  std::string body;
  std::map<std::string, std::string> headers;
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

// Windows: WinHTTP with WINHTTP_ACCESS_TYPE_NO_PROXY (WSL localhost).
std::unique_ptr<HttpTransport> CreateDefaultHttpTransport();

}  // namespace sdb
}  // namespace core

#endif  // SDB_HTTP_TRANSPORT_H
