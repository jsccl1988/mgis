// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "sdbd_session.h"

#include "sdbd_link.h"

namespace core {

SdbdConnectResult ConnectSdbd(const SdbdConnectOptions& opt,
                              std::shared_ptr<sg::HttpTransport> transport) {
  SdbdConnectResult out;
  std::string base = opt.base_url;
  std::string layer = opt.layer;
  if (base.find("://") != std::string::npos &&
      (base.rfind("sdbd://", 0) == 0 || base.find('?') != std::string::npos)) {
    sg::SdbdLink link;
    if (!sg::ParseSdbdLink(base, &link)) {
      out.error = link.error.empty() ? "bad_request" : link.error;
      return out;
    }
    base = link.base_url;
    if (layer.empty()) {
      layer = link.layer;
    }
  }

  sg::SdbdClient client(base, std::move(transport));
  const sg::SdbdResult health = client.Health();
  if (!health.ok) {
    out.error = health.error.empty() ? "not_ready" : health.error;
    return out;
  }

  const sg::SdbdResult catalog = client.ListCatalog();
  if (!catalog.ok) {
    out.error = catalog.error.empty() ? "bad_request" : catalog.error;
    return out;
  }
  sg::SdbdClient::ParseCatalog(catalog.body, &out.catalog);

  if (layer.empty()) {
    out.ok = true;
    return out;
  }

  sg::OpenRequest req = opt.open;
  req.layer = layer;
  if (req.predicate.empty()) {
    req.predicate = "intersects";
  }
  const sg::SdbdResult q = client.QueryBBox(req);
  if (!q.ok) {
    out.error = q.error.empty() ? "bad_request" : q.error;
    return out;
  }
  if (!sg::feature_set::FeatureSet::FromSdbdJson(q.body, &out.features)) {
    out.error = "bad_request";
    return out;
  }
  out.ok = true;
  return out;
}

}  // namespace core
