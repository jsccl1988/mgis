// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/sdbd_link.h"

#include "base/logging.h"
#include "core/sdb/sdbd_client.h"
#include "core/sdb/sdbd_ogr_mem.h"
#include "core/sdb/sdbd_url.h"
#include "gfx/2d/renderer/render_device.h"

#include <utility>
#include <vector>

namespace content {
namespace {

struct SdbdSession {
  std::string error;
  std::string message;
  core::sdb::LayerInfo layer_info;
  core::sdb::FeatureSet features;
  core::sdb::SdbdOgrMem mem;
  std::shared_ptr<core::sdb::HttpTransport> transport;
  std::unique_ptr<core::sdb::SdbdClient> client;
};

SdbdSession& Session() {
  static SdbdSession session;
  return session;
}

void SetError(const std::string& code, const std::string& message) {
  Session().error = code;
  Session().message = message;
  LOG(ERROR) << "sdbd: " << code
             << (message.empty() ? "" : (std::string(" ") + message));
}

bool OpenWith(const std::string& url,
              std::shared_ptr<core::sdb::HttpTransport> transport) {
  CloseSdbdLink();
  core::sdb::SdbdLinkUrl parsed = core::sdb::parse_sdbd_url(url);
  if (!parsed.ok) {
    SetError(parsed.is_mapd_port ? "bad_request" : parsed.error,
             parsed.error);
    return false;
  }

  if (!transport) {
    transport = core::sdb::CreateDefaultHttpTransport();
  }
  Session().transport = transport;
  Session().client =
      std::make_unique<core::sdb::SdbdClient>(parsed.base_url, transport);

  core::sdb::SdbdResult health = Session().client->Health();
  if (!health.ok) {
    SetError("not_ready", health.message.empty() ? "sdbd not ready"
                                                 : health.message);
    return false;
  }

  std::string err;
  std::vector<core::sdb::LayerInfo> catalog =
      Session().client->ListCatalog(&err);
  if (!err.empty() && catalog.empty()) {
    SetError(err, "catalog failed");
    return false;
  }

  std::string layer = parsed.layer;
  if (layer.empty()) {
    if (catalog.empty()) {
      // health 200 + empty catalog = link success, nothing to Bind.
      LOG(INFO) << "sdbd: catalog empty";
      return true;
    }
    layer = catalog.front().name;
  }

  Session().layer_info = Session().client->LayerInfoOf(layer, &err);
  if (Session().layer_info.name.empty()) {
    for (const auto& item : catalog) {
      if (item.name == layer) {
        Session().layer_info = item;
        err.clear();
        break;
      }
    }
  }
  if (Session().layer_info.name.empty()) {
    SetError(err.empty() ? "layer_not_found" : err, layer);
    return false;
  }

  core::sdb::OpenRequest req;
  req.layer = layer;
  req.columns = {"id", "name", "geom"};
  req.bbox = Session().layer_info.extent;
  req.predicate = "intersects";
  req.crs = Session().layer_info.crs;
  req.limit = 10000;

  err.clear();
  Session().features = Session().client->QueryBBox(req, &err);
  if (!err.empty()) {
    SetError(err, "recordset/query failed");
    return false;
  }

  if (!Session().mem.Load(Session().features, &Session().layer_info, &err)) {
    SetError(err.empty() ? "bad_request" : err, "WKT to OGR MEM failed");
    return false;
  }

  LOG(INFO) << "sdbd: opened " << parsed.base_url << " layer=" << layer
            << " features=" << Session().features.GetFeatureCount()
            << " limit_policy=reject";
  return true;
}

}  // namespace

std::string DefaultSdbdBaseUrl() { return core::sdb::default_sdbd_base(); }

bool TryOpenSdbdLink(const std::string& url) {
  return OpenWith(url, nullptr);
}

bool TryOpenSdbdLink(
    const std::string& url,
    std::shared_ptr<core::sdb::HttpTransport> transport) {
  return OpenWith(url, std::move(transport));
}

const char* SdbdLinkError() { return Session().error.c_str(); }

const char* SdbdLinkErrorMessage() { return Session().message.c_str(); }

OGRLayer* SdbdLinkLayer() { return Session().mem.layer(); }

GDALDataset* SdbdLinkDataset() { return Session().mem.dataset(); }

const core::sdb::FeatureSet* SdbdLinkFeatures() {
  return &Session().features;
}

const core::sdb::LayerInfo* SdbdLinkLayerInfo() {
  return Session().layer_info.name.empty() ? nullptr : &Session().layer_info;
}

bool BindSdbdLinkLayer(gfx2d::RenderDevice* device) {
  if (!device) {
    return false;
  }
  OGRLayer* layer = SdbdLinkLayer();
  if (!layer) {
    return device->Bind({}) == 0;
  }
  return device->Bind(std::vector<OGRLayer*>{layer}) == 0;
}

void CloseSdbdLink() {
  SdbdSession& session = Session();
  session.error.clear();
  session.message.clear();
  session.layer_info = core::sdb::LayerInfo{};
  session.features = core::sdb::FeatureSet{};
  session.mem = core::sdb::SdbdOgrMem();
  session.transport.reset();
  session.client.reset();
}

}  // namespace content
