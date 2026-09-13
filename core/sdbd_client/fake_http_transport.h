// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// 待 pin 替换: in-process fake HTTP for contract tests (no sockets).

#pragma once

#include "http_transport.h"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace sg {

class FakeHttpTransport final : public HttpTransport {
 public:
  using Handler = std::function<HttpResponse(const HttpRequest&)>;

  void set_handler(std::string path_suffix, Handler handler);
  HttpResponse Send(const HttpRequest& req) override;

  int count_q() const;
  int count_r() const;
  int count_meta() const;

 private:
  mutable std::mutex mu_;
  std::unordered_map<std::string, Handler> handlers_;
  int count_q_ = 0;
  int count_r_ = 0;
  int count_meta_ = 0;
};

}  // namespace sg
