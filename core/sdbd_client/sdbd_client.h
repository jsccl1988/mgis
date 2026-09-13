// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: target sg::SdbdClient (04 §3 / §7). HTTP first, no SQL, no GDAL.

#pragma once

#include "http_transport.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sg {

struct BBox {
  double min_x = 0;
  double min_y = 0;
  double max_x = 0;
  double max_y = 0;
};

struct FieldInfo {
  std::string name;
  std::string type;
};

struct LayerInfo {
  std::string name;
  std::string crs;
  std::string geom_type;
  std::string geom_column;
  BBox extent;
  std::vector<FieldInfo> fields;
};

struct OpenRequest {
  std::string layer;
  std::vector<std::string> columns;
  std::optional<BBox> bbox;
  std::string predicate = "intersects";
  std::string crs;
  std::optional<int> limit;

  // Object bbox {min_x,...}. Never emits sql / raw_sql / raw_query.
  std::string ToJson() const;
};

struct SdbdResult {
  bool ok = false;
  int status = 0;
  std::string body;
  std::string error;
};

class SdbdClient {
 public:
  SdbdClient(std::string base_url, std::shared_ptr<HttpTransport> transport);

  const std::string& base_url() const { return base_url_; }

  SdbdResult Health();
  SdbdResult ListCatalog();
  SdbdResult LayerInfoOf(const std::string& layer);
  SdbdResult QueryBBox(const OpenRequest& req);
  SdbdResult Open(const OpenRequest& req);
  SdbdResult Fetch(const std::string& handle, int offset, int limit);
  SdbdResult Close(const std::string& handle);

  static bool ParseLayerInfo(const std::string& json, LayerInfo* out);
  static bool ParseCatalog(const std::string& json, std::vector<LayerInfo>* out);
  // Empty string = allowed. "forbidden_sql" if body has sql / raw_sql / raw_query.
  static std::string CheckRequestBody(const std::string& json);

 private:
  SdbdResult Send(HttpRequest req);
  SdbdResult PostJson(const std::string& path, const std::string& body);

  std::string base_url_;
  std::shared_ptr<HttpTransport> transport_;
};

}  // namespace sg
