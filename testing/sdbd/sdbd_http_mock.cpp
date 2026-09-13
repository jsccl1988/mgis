// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "testing/sdbd/sdbd_http_mock.h"

#include "httplib.h"

#include <chrono>

namespace testing {
namespace sdbd {

class SdbdHttpMock::Impl {
 public:
  httplib::Server server;
};

SdbdHttpMock::SdbdHttpMock() : impl_(std::make_unique<Impl>()) {
  auto dispatch = [this](const httplib::Request& req, httplib::Response& res) {
    int status = 500;
    std::string body;
    service_.Handle(req.method, req.path, req.body, &status, &body);
    res.status = status;
    res.set_content(body, "application/json");
  };
  impl_->server.Get("/sdbd/api/v1/health", dispatch);
  impl_->server.Get("/sdbd/api/v1/catalog", dispatch);
  impl_->server.Get(R"(/sdbd/api/v1/layers/(.+))", dispatch);
  impl_->server.Post("/sdbd/api/v1/recordset/open", dispatch);
  impl_->server.Post("/sdbd/api/v1/recordset/fetch", dispatch);
  impl_->server.Post("/sdbd/api/v1/recordset/close", dispatch);
  impl_->server.Post("/sdbd/api/v1/recordset/query", dispatch);
}

SdbdHttpMock::~SdbdHttpMock() { Stop(); }

bool SdbdHttpMock::Start(const std::string& host, int port) {
  if (!impl_->server.bind_to_port(host.c_str(), port)) {
    return false;
  }
  port_ = port;
  thread_ = std::thread([this] { impl_->server.listen_after_bind(); });
  for (int i = 0; i < 50 && !impl_->server.is_running(); ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return impl_->server.is_running();
}

bool SdbdHttpMock::StartAny() {
  const int port = impl_->server.bind_to_any_port("127.0.0.1");
  if (port <= 0) {
    return false;
  }
  port_ = port;
  thread_ = std::thread([this] { impl_->server.listen_after_bind(); });
  for (int i = 0; i < 50 && !impl_->server.is_running(); ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return impl_->server.is_running();
}

void SdbdHttpMock::Stop() {
  if (impl_ && impl_->server.is_running()) {
    impl_->server.stop();
  }
  if (thread_.joinable()) {
    thread_.join();
  }
  port_ = 0;
}

std::string SdbdHttpMock::base_url() const {
  return "http://127.0.0.1:" + std::to_string(port_);
}

}  // namespace sdbd
}  // namespace testing
