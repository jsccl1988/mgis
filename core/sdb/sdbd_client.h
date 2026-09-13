// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef SDB_SDBD_CLIENT_H
#define SDB_SDBD_CLIENT_H

#include "core/sdb/http_transport.h"
#include "core/sdb/sdbd_types.h"

#include <memory>
#include <string>
#include <vector>

namespace core {
namespace sdb {

// Thin HTTP client for /sdbd/api/v1/* (04 §3). No SQL. No mapd Path Q.
class SdbdClient {
 public:
  SdbdClient(std::string base_url, std::shared_ptr<HttpTransport> transport);

  const std::string& base_url() const { return base_url_; }

  SdbdResult Health();
  std::vector<LayerInfo> ListCatalog(std::string* err);
  LayerInfo LayerInfoOf(const std::string& layer, std::string* err);
  FeatureSet QueryBBox(const OpenRequest& req, std::string* err);

  SdbdResult RecordsetOpen(const OpenRequest& req);
  FeatureSet RecordsetFetch(const std::string& handle, int offset, int limit,
                            std::string* err);
  SdbdResult RecordsetClose(const std::string& handle);

 private:
  SdbdResult Send(HttpRequest req);
  SdbdResult Get(const std::string& path);
  SdbdResult Post(const std::string& path, const std::string& body);

  std::string base_url_;
  std::shared_ptr<HttpTransport> transport_;
};

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_CLIENT_H
