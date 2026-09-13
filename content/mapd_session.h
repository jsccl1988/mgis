// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_MAPD_SESSION_H
#define CONTENT_MAPD_SESSION_H

#include "content/content_export.h"
#include "mapd_client.h"

#include <memory>
#include <string>

#include "ogrsf_frmts.h"

namespace content {

class CONTENT_EXPORT MapdSession {
 public:
  MapdSession() = default;
  ~MapdSession();

  MapdSession(const MapdSession&) = delete;
  MapdSession& operator=(const MapdSession&) = delete;

  bool Open(const std::string& url,
            std::shared_ptr<sg::HttpTransport> transport = nullptr);
  void Reset();

  const std::string& error() const { return error_; }
  const std::string& base_url() const { return base_url_; }
  const std::string& layer() const { return layer_; }
  const std::string& style_json() const { return style_json_; }
  const sg::Capabilities& capabilities() const { return capabilities_; }
  GDALDataset* dataset() const { return dataset_; }
  GDALDataset* ReleaseDataset();
  int LayerCount() const;

 private:
  bool GeoJsonToMem(const std::string& geojson);

  std::string error_;
  std::string base_url_;
  std::string layer_;
  std::string style_json_;
  sg::Capabilities capabilities_;
  GDALDataset* dataset_{nullptr};
};

CONTENT_EXPORT MapdSession& SharedMapdSession();

}  // namespace content

#endif  // CONTENT_MAPD_SESSION_H
