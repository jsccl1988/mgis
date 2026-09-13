// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "mapd_client.h"

#include "json_lite.h"

#include <sstream>

namespace sg {
namespace {

std::string TrimTrailingSlash(std::string s) {
  while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
    s.pop_back();
  }
  return s;
}

}  // namespace

void DualPathQueue::Enqueue(Kind kind, std::string label) {
  std::lock_guard<std::mutex> lock(mu_);
  Item item;
  item.kind = kind;
  item.label = std::move(label);
  if (kind == Kind::kQ) {
    q_.push(std::move(item));
  } else {
    r_.push(std::move(item));
  }
}

bool DualPathQueue::TryDequeueR(Item* out) {
  std::lock_guard<std::mutex> lock(mu_);
  if (r_.empty() || !out) {
    return false;
  }
  *out = std::move(r_.front());
  r_.pop();
  return true;
}

bool DualPathQueue::TryDequeueQ(Item* out) {
  std::lock_guard<std::mutex> lock(mu_);
  if (q_.empty() || !out) {
    return false;
  }
  *out = std::move(q_.front());
  q_.pop();
  return true;
}

std::size_t DualPathQueue::pending_q() const {
  std::lock_guard<std::mutex> lock(mu_);
  return q_.size();
}

std::size_t DualPathQueue::pending_r() const {
  std::lock_guard<std::mutex> lock(mu_);
  return r_.size();
}

MapdClient::MapdClient(std::string base_url,
                       std::shared_ptr<HttpTransport> transport)
    : base_url_(TrimTrailingSlash(std::move(base_url))),
      transport_(std::move(transport)) {}

MapdResult MapdClient::Send(HttpRequest req) {
  MapdResult out;
  if (!transport_) {
    out.error = "not_ready";
    return out;
  }
  HttpResponse res = transport_->Send(req);
  if (!res.error.empty()) {
    out.error = "not_ready";
    out.status = res.status;
    return out;
  }
  out.status = res.status;
  out.body = std::move(res.body);
  out.ok = res.status >= 200 && res.status < 300;
  if (!out.ok) {
    out.error = (res.status == 503) ? "not_ready"
                                    : ("HTTP " + std::to_string(res.status));
  }
  return out;
}

MapdResult MapdClient::Health() {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/mapd/api/v1/health";
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  MapdResult r = Send(std::move(req));
  if (!r.ok && r.error.find("HTTP") == 0) {
    r.error = "not_ready";
  }
  return r;
}

MapdResult MapdClient::GetStyle() {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/mapd/style.json";
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  return Send(std::move(req));
}

MapdResult MapdClient::GetCapabilities() {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/mapd/api/v1/capabilities";
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  return Send(std::move(req));
}

MapdResult MapdClient::Query(const QueryRequest& req) {
  HttpRequest http;
  http.method = "POST";
  http.url = base_url_ + "/mapd/api/v1/features/query";
  http.path_class = HttpRequest::PathClass::kQ;
  http.headers["Content-Type"] = "application/json";
  http.headers["Accept"] = "application/json";
  http.headers["Cache-Control"] = "no-store";
  http.body = req.ToJson();
  queues_.Enqueue(DualPathQueue::Kind::kQ, "features/query");
  return Send(std::move(http));
}

std::string MapdClient::TileUrl(TileKind kind, int z, int x, int y) const {
  std::ostringstream o;
  o << base_url_ << "/mapd/tiles/";
  switch (kind) {
    case TileKind::kRaster:
      o << "raster/" << z << "/" << x << "/" << y << ".webp";
      break;
    case TileKind::kVector:
      o << "vector/" << z << "/" << x << "/" << y << ".mvt";
      break;
    case TileKind::kTerrain:
      o << "terrain/" << z << "/" << x << "/" << y << ".png";
      break;
    case TileKind::kSatellite:
      o << "satellite/" << z << "/" << x << "/" << y;
      break;
  }
  return o.str();
}

bool MapdClient::ParseCapabilities(const std::string& json, Capabilities* out) {
  if (!out) {
    return false;
  }
  json::Value root;
  if (!json::Parse(json, &root, nullptr) || !root.is_object()) {
    return false;
  }
  out->crs = root.String("crs");
  out->data_version = root.String("data_version");
  if (const json::Value* path_q = root.Get("path_q")) {
    out->path_q_features = path_q->Bool("features", false);
    out->path_q_analysis = path_q->Bool("analysis", false);
    out->postgis_configured = path_q->Bool("postgis_configured", false);
    out->primary_store = path_q->String("primary_store");
  }
  if (out->primary_store.empty()) {
    out->primary_store = out->postgis_configured ? "postgis" : "table";
  }
  if (const json::Value* path_r = root.Get("path_r")) {
    out->path_r_raster = path_r->Bool("raster", false);
    out->path_r_vector = path_r->Bool("vector", false);
  }
  out->query_layers.clear();
  if (const json::Value* ql = root.Get("query_layers")) {
    if (ql->is_array()) {
      for (const auto& item : ql->array) {
        if (item.type == json::Type::kString) {
          out->query_layers.push_back(item.str);
        }
      }
    }
  }
  return !out->crs.empty() || root.Get("service") != nullptr;
}

}  // namespace sg
