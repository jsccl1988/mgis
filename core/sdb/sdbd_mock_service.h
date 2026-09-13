// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// In-process sdbd contract mock (not a mogu sdbd product).

#ifndef SDB_SDBD_MOCK_SERVICE_H
#define SDB_SDBD_MOCK_SERVICE_H

#include "core/sdb/fake_http_transport.h"
#include "core/sdb/sdbd_types.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace core {
namespace sdb {

struct MockFeature {
  Feature feature;
  BBox mbr;
};

class SdbdMockService {
 public:
  SdbdMockService();

  void set_ready(bool ready) { ready_ = ready; }
  void Handle(const std::string& method, const std::string& path,
              const std::string& body, int* status, std::string* response);

  // Wire health/catalog/layer.info/recordset.* onto a fake transport.
  void Install(FakeHttpTransport* transport);

  static LayerInfo DemoLayerInfo();
  static std::vector<MockFeature> DemoFeatures();

 private:
  struct Cursor {
    std::string layer;
    std::string crs;
    std::vector<std::size_t> hits;
  };

  void WriteError(int http, const char* code, const char* message, int* status,
                  std::string* response) const;
  bool ParseOpen(const std::string& body, OpenRequest* req, int* status,
                 std::string* response) const;
  std::vector<std::size_t> QueryHits(const OpenRequest& req) const;
  std::string FeatureSetJson(const std::string& layer, const std::string& crs,
                             int offset,
                             const std::vector<std::size_t>& hits) const;
  std::string LayerInfoJson(const LayerInfo& info) const;

  bool ready_ = true;
  std::uint64_t next_handle_ = 1;
  std::map<std::string, Cursor> cursors_;
  std::vector<MockFeature> features_;
  LayerInfo layer_;
};

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_MOCK_SERVICE_H
