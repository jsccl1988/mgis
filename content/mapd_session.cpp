// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "content/mapd_session.h"

#include "base/logging.h"
#include "content/mapd_link.h"
#include "core/mapd_client/mapd_link.h"

#include <cpl_conv.h>
#include <cpl_vsi.h>

#include <cstring>

namespace content {
namespace {

GDALDataset* OpenGeoJsonMem(const std::string& geojson) {
  const std::string vsi_path = "/vsimem/mgis_mapd_path_q.geojson";
  GByte* buf = static_cast<GByte*>(CPLMalloc(geojson.size() + 1));
  std::memcpy(buf, geojson.data(), geojson.size());
  buf[geojson.size()] = 0;
  VSILFILE* fp = VSIFileFromMemBuffer(vsi_path.c_str(), buf,
                                      static_cast<vsi_l_offset>(geojson.size()),
                                      TRUE);
  if (fp) {
    VSIFCloseL(fp);
  }

  GDALDataset* src = static_cast<GDALDataset*>(GDALOpenEx(
      vsi_path.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, nullptr,
      nullptr));
  if (!src) {
    VSIUnlink(vsi_path.c_str());
    return nullptr;
  }

  GDALDriver* mem = GetGDALDriverManager()->GetDriverByName("Memory");
  GDALDataset* dst = nullptr;
  if (mem) {
    dst = mem->CreateCopy("mapd_path_q", src, FALSE, nullptr, nullptr, nullptr);
  }
  GDALClose(src);
  VSIUnlink(vsi_path.c_str());
  return dst;
}

}  // namespace

MapdSession::~MapdSession() { Reset(); }

void MapdSession::Reset() {
  if (dataset_) {
    GDALClose(dataset_);
    dataset_ = nullptr;
  }
  error_.clear();
  base_url_.clear();
  layer_.clear();
  style_json_.clear();
  capabilities_ = sg::Capabilities{};
}

GDALDataset* MapdSession::ReleaseDataset() {
  GDALDataset* ds = dataset_;
  dataset_ = nullptr;
  return ds;
}

int MapdSession::LayerCount() const {
  return dataset_ ? dataset_->GetLayerCount() : 0;
}

bool MapdSession::GeoJsonToMem(const std::string& geojson) {
  if (dataset_) {
    GDALClose(dataset_);
    dataset_ = nullptr;
  }
  dataset_ = OpenGeoJsonMem(geojson);
  return dataset_ != nullptr;
}

bool MapdSession::Open(const std::string& url,
                       std::shared_ptr<sg::HttpTransport> transport) {
  Reset();
  GDALAllRegister();
  sg::MapdLink link;
  if (!sg::ParseMapdLink(url, &link)) {
    error_ = link.error.empty() ? "bad_request" : link.error;
    LOG(ERROR) << "MapdSession: bad link url=" << url << " err=" << error_;
    return false;
  }
  base_url_ = link.base_url;
  layer_ = link.layer;

  if (!transport) {
    transport =
        std::shared_ptr<sg::HttpTransport>(sg::CreateDefaultHttpTransport());
  }
  sg::MapdClient client(base_url_, transport);

  const sg::MapdResult health = client.Health();
  if (!health.ok) {
    error_ = "not_ready";
    LOG(ERROR) << "mapd 未就绪 not_ready url=" << base_url_
               << " status=" << health.status << " err=" << health.error;
    return false;
  }

  const sg::MapdResult caps = client.GetCapabilities();
  if (caps.ok) {
    sg::MapdClient::ParseCapabilities(caps.body, &capabilities_);
  } else {
    LOG(WARNING) << "MapdSession: capabilities failed status=" << caps.status
                 << " err=" << caps.error;
  }

  const sg::MapdResult style = client.GetStyle();
  if (style.ok) {
    style_json_ = style.body;
  }

  if (layer_.empty() && !capabilities_.query_layers.empty()) {
    layer_ = capabilities_.query_layers.front();
  }
  if (layer_.empty()) {
    layer_ = "basemap";
  }

  if (capabilities_.path_q_features || caps.ok) {
    sg::QueryRequest req;
    req.layer = layer_;
    req.bbox = {{116.3, 39.8, 116.5, 40.0}};
    req.limit = 100;
    LOG(INFO) << "MapdSession: Path Q layer=" << layer_
              << " limit_policy=clamp";
    const sg::MapdResult query = client.Query(req);
    if (query.ok && query.body.find("FeatureCollection") != std::string::npos) {
      if (!GeoJsonToMem(query.body)) {
        LOG(WARNING) << "MapdSession: GeoJSON MEM bind failed gdal="
                     << CPLGetLastErrorMsg();
      }
    } else if (!query.ok) {
      LOG(WARNING) << "MapdSession: Path Q failed status=" << query.status
                   << " err=" << query.error << " (no shp fallback)";
    }
  } else {
    LOG(INFO) << "MapdSession: no path_q.features — 仅底图, no Path Q";
  }

  return true;
}

MapdSession& SharedMapdSession() {
  static MapdSession session;
  return session;
}

}  // namespace content
