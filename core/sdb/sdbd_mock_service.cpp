// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "core/sdb/sdbd_mock_service.h"

#include "core/sdb/sdbd_json.h"
#include "http_path.h"

#include <sstream>

namespace core {
namespace sdb {
namespace {

std::string PathOnly(const std::string& path) {
  const auto q = path.find('?');
  return q == std::string::npos ? path : path.substr(0, q);
}

}  // namespace

LayerInfo SdbdMockService::DemoLayerInfo() {
  LayerInfo info;
  info.name = kDemoLayer;
  info.crs = kDemoCrs;
  info.geom_type = "polygon";
  info.geom_column = "geom";
  info.extent = {0, 0, 1000, 1000};
  info.fields = {{"id", "uint64"}, {"name", "string"}};
  return info;
}

std::vector<MockFeature> SdbdMockService::DemoFeatures() {
  // Golden: ≥3 polygons. Narrow (100,100)-(200,200) → count==1 id==2.
  return {
      {{"1", "POLYGON((0 0,50 0,50 50,0 50,0 0))", {{"name", "a"}}},
       {0, 0, 50, 50}},
      {{"2", "POLYGON((120 120,180 120,180 180,120 180,120 120))",
        {{"name", "b"}}},
       {120, 120, 180, 180}},
      {{"3", "POLYGON((800 800,900 800,900 900,800 900,800 800))",
        {{"name", "c"}}},
       {800, 800, 900, 900}},
  };
}

SdbdMockService::SdbdMockService()
    : features_(DemoFeatures()), layer_(DemoLayerInfo()) {}

void SdbdMockService::WriteError(int http, const char* code,
                                 const char* message, int* status,
                                 std::string* response) const {
  if (status) {
    *status = http;
  }
  if (response) {
    *response = std::string("{\"ok\":false,\"error\":\"") + code +
                "\",\"message\":\"" + JsonEscape(message) + "\"}";
  }
}

std::string SdbdMockService::LayerInfoJson(const LayerInfo& info) const {
  std::ostringstream o;
  o << "{\"name\":\"" << JsonEscape(info.name) << "\",\"crs\":\""
    << JsonEscape(info.crs) << "\",\"geom_type\":\""
    << JsonEscape(info.geom_type) << "\",\"geom_column\":\""
    << JsonEscape(info.geom_column) << "\",\"extent\":{\"min_x\":"
    << info.extent.min_x << ",\"min_y\":" << info.extent.min_y << ",\"max_x\":"
    << info.extent.max_x << ",\"max_y\":" << info.extent.max_y
    << "},\"fields\":[";
  for (std::size_t i = 0; i < info.fields.size(); ++i) {
    if (i) {
      o << ',';
    }
    o << "{\"name\":\"" << JsonEscape(info.fields[i].name) << "\",\"type\":\""
      << JsonEscape(info.fields[i].type) << "\"}";
  }
  o << "]}";
  return o.str();
}

std::string SdbdMockService::FeatureSetJson(
    const std::string& layer, const std::string& crs, int offset,
    const std::vector<std::size_t>& hits) const {
  std::ostringstream o;
  o << "{\"layer\":\"" << JsonEscape(layer) << "\",\"crs\":\""
    << JsonEscape(crs) << "\",\"count\":" << hits.size() << ",\"offset\":"
    << offset << ",\"features\":[";
  for (std::size_t i = 0; i < hits.size(); ++i) {
    if (i) {
      o << ',';
    }
    const Feature& f = features_[hits[i]].feature;
    o << "{\"id\":" << f.id << ",\"geom_wkt\":\"" << JsonEscape(f.geom_wkt)
      << "\",\"attrs\":{";
    bool first = true;
    for (const auto& kv : f.attrs) {
      if (!first) {
        o << ',';
      }
      first = false;
      o << '"' << JsonEscape(kv.first) << "\":\"" << JsonEscape(kv.second)
        << '"';
    }
    o << "}}";
  }
  o << "]}";
  return o.str();
}

bool SdbdMockService::ParseOpen(const std::string& body, OpenRequest* req,
                                int* status, std::string* response) const {
  if (BodyHasForbiddenSql(body)) {
    WriteError(400, "forbidden_sql", "sql keys are not allowed", status,
               response);
    return false;
  }
  Json json;
  std::string err;
  if (!ParseJson(body, &json, &err) || json.type != Json::kObject) {
    WriteError(400, "bad_request", "invalid JSON", status, response);
    return false;
  }
  req->layer = json.String("layer");
  if (req->layer.empty()) {
    WriteError(400, "bad_request", "missing layer", status, response);
    return false;
  }
  if (const Json* columns = json.Get("columns")) {
    if (columns->type == Json::kArray) {
      for (const auto& c : columns->a) {
        if (c.type == Json::kString) {
          req->columns.push_back(c.s);
        }
      }
    }
  }
  if (const Json* bbox = json.Get("bbox")) {
    req->bbox.min_x = bbox->Number("min_x");
    req->bbox.min_y = bbox->Number("min_y");
    req->bbox.max_x = bbox->Number("max_x");
    req->bbox.max_y = bbox->Number("max_y");
    if (req->bbox.max_x < req->bbox.min_x ||
        req->bbox.max_y < req->bbox.min_y) {
      WriteError(400, "bad_request", "invalid bbox", status, response);
      return false;
    }
  } else {
    req->bbox = layer_.extent;
  }
  req->predicate = json.String("predicate", "intersects");
  req->crs = json.String("crs");
  if (json.Has("limit")) {
    req->limit = static_cast<int>(json.Number("limit"));
  }
  if (!req->predicate.empty() && req->predicate != "intersects") {
    WriteError(400, "unsupported_predicate", "only intersects is supported",
               status, response);
    return false;
  }
  if (!req->crs.empty() && req->crs != layer_.crs) {
    WriteError(400, "bad_request", "crs mismatch (no reproject)", status,
               response);
    return false;
  }
  if (req->limit > kLimitHardCap) {
    WriteError(400, "limit_exceeded", "limit exceeds hard cap", status,
               response);
    return false;
  }
  return true;
}

std::vector<std::size_t> SdbdMockService::QueryHits(
    const OpenRequest& req) const {
  std::vector<std::size_t> hits;
  for (std::size_t i = 0; i < features_.size(); ++i) {
    if (features_[i].mbr.Intersects(req.bbox)) {
      hits.push_back(i);
    }
  }
  if (req.limit > 0 && static_cast<int>(hits.size()) > req.limit) {
    hits.resize(static_cast<std::size_t>(req.limit));
  }
  return hits;
}

void SdbdMockService::Handle(const std::string& method, const std::string& raw,
                             const std::string& body, int* status,
                             std::string* response) {
  const std::string path = PathOnly(raw);
  if (!ready_) {
    WriteError(503, "not_ready", "sdbd engine not loaded", status, response);
    return;
  }

  if (method == "GET" && path == "/sdbd/api/v1/health") {
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = "{\"ok\":true,\"service\":\"sdbd\"}";
    }
    return;
  }

  if (method == "GET" && path == "/sdbd/api/v1/catalog") {
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = "[" + LayerInfoJson(layer_) + "]";
    }
    return;
  }

  const std::string layer_prefix = "/sdbd/api/v1/layers/";
  if (method == "GET" && path.rfind(layer_prefix, 0) == 0) {
    const std::string name = path.substr(layer_prefix.size());
    if (name != layer_.name) {
      WriteError(404, "layer_not_found", "unknown layer", status, response);
      return;
    }
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = LayerInfoJson(layer_);
    }
    return;
  }

  if (method != "POST") {
    WriteError(400, "bad_request", "unsupported method", status, response);
    return;
  }

  if (path == "/sdbd/api/v1/recordset/open" ||
      path == "/sdbd/api/v1/recordset/query") {
    OpenRequest req;
    if (!ParseOpen(body, &req, status, response)) {
      return;
    }
    if (req.layer != layer_.name) {
      WriteError(404, "layer_not_found", "unknown layer", status, response);
      return;
    }
    const auto hits = QueryHits(req);
    if (path == "/sdbd/api/v1/recordset/query") {
      if (status) {
        *status = 200;
      }
      if (response) {
        *response = FeatureSetJson(req.layer, layer_.crs, 0, hits);
      }
      return;
    }
    const std::string handle = std::to_string(next_handle_++);
    cursors_[handle] = Cursor{req.layer, layer_.crs, hits};
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = std::string("{\"ok\":true,\"handle\":\"") + handle +
                  "\",\"schema\":" + LayerInfoJson(layer_) + ",\"matched\":" +
                  std::to_string(hits.size()) + "}";
    }
    return;
  }

  if (path == "/sdbd/api/v1/recordset/fetch") {
    if (BodyHasForbiddenSql(body)) {
      WriteError(400, "forbidden_sql", "sql keys are not allowed", status,
                 response);
      return;
    }
    Json json;
    if (!ParseJson(body, &json, nullptr) || json.type != Json::kObject) {
      WriteError(400, "bad_request", "invalid JSON", status, response);
      return;
    }
    const std::string handle = json.String("handle");
    auto it = cursors_.find(handle);
    if (it == cursors_.end()) {
      WriteError(404, "handle_not_found", "unknown handle", status, response);
      return;
    }
    int offset = static_cast<int>(json.Number("offset"));
    int limit = json.Has("limit") ? static_cast<int>(json.Number("limit"))
                                  : kFetchDefault;
    if (limit > kFetchMax || limit > kLimitHardCap) {
      WriteError(400, "limit_exceeded", "fetch limit exceeds cap", status,
                 response);
      return;
    }
    if (offset < 0) {
      offset = 0;
    }
    std::vector<std::size_t> slice;
    for (int i = offset;
         i < static_cast<int>(it->second.hits.size()) && i < offset + limit;
         ++i) {
      slice.push_back(it->second.hits[static_cast<std::size_t>(i)]);
    }
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = FeatureSetJson(it->second.layer, it->second.crs, offset, slice);
    }
    return;
  }

  if (path == "/sdbd/api/v1/recordset/close") {
    Json json;
    if (!ParseJson(body, &json, nullptr) || json.type != Json::kObject) {
      WriteError(400, "bad_request", "invalid JSON", status, response);
      return;
    }
    const std::string handle = json.String("handle");
    if (cursors_.erase(handle) == 0) {
      WriteError(404, "handle_not_found", "unknown handle", status, response);
      return;
    }
    if (status) {
      *status = 200;
    }
    if (response) {
      *response = "{\"ok\":true}";
    }
    return;
  }

  WriteError(404, "layer_not_found", "unknown route", status, response);
}

void SdbdMockService::Install(FakeHttpTransport* transport) {
  if (!transport) {
    return;
  }
  auto handler = [this](const HttpRequest& req) {
    HttpResponse res;
    const std::string path = sg::detail::normalize_http_path(req.url);
    Handle(req.method, path, req.body, &res.status, &res.body);
    return res;
  };
  transport->set_handler("/sdbd/api/v1/", handler);
}

}  // namespace sdb
}  // namespace core
