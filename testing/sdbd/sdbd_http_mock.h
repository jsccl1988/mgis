// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// Tiny httplib listener wrapping SdbdMockService. Not a mogu sdbd product.

#ifndef TESTING_SDBD_HTTP_MOCK_H
#define TESTING_SDBD_HTTP_MOCK_H

#include "core/sdb/sdbd_mock_service.h"

#include <memory>
#include <string>
#include <thread>

namespace testing {
namespace sdbd {

class SdbdHttpMock {
 public:
  SdbdHttpMock();
  ~SdbdHttpMock();

  bool Start(const std::string& host, int port);
  bool StartAny();
  void Stop();

  int port() const { return port_; }
  std::string base_url() const;
  core::sdb::SdbdMockService& service() { return service_; }

 private:
  class Impl;

  core::sdb::SdbdMockService service_;
  std::unique_ptr<Impl> impl_;
  std::thread thread_;
  int port_ = 0;
};

}  // namespace sdbd
}  // namespace testing

#endif  // TESTING_SDBD_HTTP_MOCK_H
