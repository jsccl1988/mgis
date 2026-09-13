// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "core/sdb/http_transport.h"

#if defined(_WIN32)

#include "winhttp_send.h"

namespace core {
namespace sdb {
namespace {

class WinHttpTransport final : public HttpTransport {
 public:
  HttpResponse Send(const HttpRequest& req) override {
    return sg::winhttp_send<sg::legacy_sdb_winhttp_traits, HttpRequest,
                            HttpResponse>(req);
  }
};

}  // namespace

std::unique_ptr<HttpTransport> CreateDefaultHttpTransport() {
  return std::make_unique<WinHttpTransport>();
}

}  // namespace sdb
}  // namespace core

#else

namespace core {
namespace sdb {

namespace {

class StubHttpTransport final : public HttpTransport {
 public:
  HttpResponse Send(const HttpRequest&) override {
    HttpResponse r;
    r.error = "WinHTTP transport is Windows-only; inject FakeHttpTransport";
    return r;
  }
};

}  // namespace

std::unique_ptr<HttpTransport> CreateDefaultHttpTransport() {
  return std::make_unique<StubHttpTransport>();
}

}  // namespace sdb
}  // namespace core

#endif
