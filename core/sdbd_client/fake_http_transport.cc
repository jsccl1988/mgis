// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "fake_http_transport.h"
#include "http_path.h"

namespace sg {

void FakeHttpTransport::set_handler(std::string path_suffix, Handler handler) {
  std::lock_guard<std::mutex> lock(mu_);
  handlers_[std::move(path_suffix)] = std::move(handler);
}

HttpResponse FakeHttpTransport::Send(const HttpRequest& req) {
  {
    std::lock_guard<std::mutex> lock(mu_);
    switch (req.path_class) {
      case HttpRequest::PathClass::kQ:
        ++count_q_;
        break;
      case HttpRequest::PathClass::kR:
        ++count_r_;
        break;
      default:
        ++count_meta_;
        break;
    }
  }

  const std::string path = detail::normalize_http_path(req.url);
  Handler handler;
  {
    std::lock_guard<std::mutex> lock(mu_);
    handler = detail::find_http_handler<detail::suffix_or_contains_match>(
        handlers_, path);
  }
  if (!handler) {
    HttpResponse res;
    res.status = 404;
    res.body = "{\"error\":\"not found\"}";
    return res;
  }
  return handler(req);
}

int FakeHttpTransport::count_q() const {
  std::lock_guard<std::mutex> lock(mu_);
  return count_q_;
}

int FakeHttpTransport::count_r() const {
  std::lock_guard<std::mutex> lock(mu_);
  return count_r_;
}

int FakeHttpTransport::count_meta() const {
  std::lock_guard<std::mutex> lock(mu_);
  return count_meta_;
}

}  // namespace sg
