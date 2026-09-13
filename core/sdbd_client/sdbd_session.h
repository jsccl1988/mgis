// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// Optional Connection / Execution / Recordset entry. Not wired into
// MapWindow::InitMap (crash-fix agents own that file). Default demo stays
// GDI + shapefile. Callers may pass MGIS_USE_SDBD later; health failure
// is not_ready and does not open a shapefile.

#pragma once

#include "http_transport.h"
#include "sdbd_client.h"
#include "feature_set.h"

#include <memory>
#include <string>
#include <vector>

namespace core {

struct SdbdConnectOptions {
  std::string base_url;
  std::string layer;
  sg::OpenRequest open;
};

struct SdbdConnectResult {
  bool ok = false;
  std::string error;
  std::vector<sg::LayerInfo> catalog;
  sg::feature_set::FeatureSet features;
};

SdbdConnectResult ConnectSdbd(const SdbdConnectOptions& opt,
                              std::shared_ptr<sg::HttpTransport> transport);

}  // namespace core
