// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "core/sdb/fake_http_transport.h"
#include "http_path.h"

namespace core {
namespace sdb {

void FakeHttpTransport::set_handler(std::string path_suffix, Handler handler) {
  std::lock_guard<std::mutex> lock(mu_);
  handlers_[std::move(path_suffix)] = std::move(handler);
}

HttpResponse FakeHttpTransport::Send(const HttpRequest& req) {
  const std::string path = sg::detail::normalize_http_path(req.url);
  Handler handler;
  {
    std::lock_guard<std::mutex> lock(mu_);
    handler = sg::detail::find_http_handler<sg::detail::prefix_or_contains_match>(
        handlers_, path);
  }
  if (!handler) {
    HttpResponse res;
    res.status = 404;
    res.body = "{\"ok\":false,\"error\":\"layer_not_found\",\"message\":\"no handler\"}";
    return res;
  }
  return handler(req);
}

}  // namespace sdb
}  // namespace core
