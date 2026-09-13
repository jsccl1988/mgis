// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// In-process fake HTTP for contract tests (no sockets).

#ifndef SDB_FAKE_HTTP_TRANSPORT_H
#define SDB_FAKE_HTTP_TRANSPORT_H

#include "core/sdb/http_transport.h"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace core {
namespace sdb {

class FakeHttpTransport final : public HttpTransport {
 public:
  using Handler = std::function<HttpResponse(const HttpRequest&)>;

  void set_handler(std::string path_suffix, Handler handler);
  HttpResponse Send(const HttpRequest& req) override;

 private:
  mutable std::mutex mu_;
  std::unordered_map<std::string, Handler> handlers_;
};

}  // namespace sdb
}  // namespace core

#endif  // SDB_FAKE_HTTP_TRANSPORT_H
