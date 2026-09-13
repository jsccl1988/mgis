// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "sdbd_client.h"

#include "json_lite.h"

#include <cstdio>
#include <sstream>

namespace sg {
namespace {

std::string TrimTrailingSlash(std::string s) {
  while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
    s.pop_back();
  }
  return s;
}

std::string EscapeJsonString(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

std::string FormatDouble(double v) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.10g", v);
  return buf;
}

std::string ErrorFromBody(const std::string& body, const std::string& fallback) {
  json::Value v;
  if (json::Parse(body, &v, nullptr)) {
    const std::string err = v.String("error");
    if (!err.empty()) {
      return err;
    }
    if (v.Get("ok") && !v.Bool("ok", true)) {
      return fallback;
    }
  }
  return fallback;
}

bool FillLayerInfo(const json::Value& obj, LayerInfo* out) {
  if (!obj.is_object() || !out) {
    return false;
  }
  out->name = obj.String("name");
  if (out->name.empty()) {
    return false;
  }
  out->crs = obj.String("crs");
  out->geom_type = obj.String("geom_type");
  out->geom_column = obj.String("geom_column");
  if (const json::Value* ext = obj.Get("extent")) {
    out->extent.min_x = ext->Number("min_x", 0);
    out->extent.min_y = ext->Number("min_y", 0);
    out->extent.max_x = ext->Number("max_x", 0);
    out->extent.max_y = ext->Number("max_y", 0);
  }
  out->fields.clear();
  if (const json::Value* fields = obj.Get("fields")) {
    if (fields->is_array()) {
      for (const auto& f : fields->array) {
        FieldInfo fi;
        fi.name = f.String("name");
        fi.type = f.String("type");
        if (!fi.name.empty()) {
          out->fields.push_back(std::move(fi));
        }
      }
    }
  }
  return true;
}

}  // namespace

std::string OpenRequest::ToJson() const {
  std::ostringstream o;
  o << "{\"layer\":\"" << EscapeJsonString(layer) << "\"";
  if (!columns.empty()) {
    o << ",\"columns\":[";
    for (std::size_t i = 0; i < columns.size(); ++i) {
      if (i) {
        o << ",";
      }
      o << "\"" << EscapeJsonString(columns[i]) << "\"";
    }
    o << "]";
  }
  if (bbox) {
    o << ",\"bbox\":{\"min_x\":" << FormatDouble(bbox->min_x) << ",\"min_y\":"
      << FormatDouble(bbox->min_y) << ",\"max_x\":" << FormatDouble(bbox->max_x)
      << ",\"max_y\":" << FormatDouble(bbox->max_y) << "}";
  }
  if (!predicate.empty()) {
    o << ",\"predicate\":\"" << EscapeJsonString(predicate) << "\"";
  }
  if (!crs.empty()) {
    o << ",\"crs\":\"" << EscapeJsonString(crs) << "\"";
  }
  if (limit) {
    o << ",\"limit\":" << *limit;
  }
  o << "}";
  return o.str();
}

SdbdClient::SdbdClient(std::string base_url,
                       std::shared_ptr<HttpTransport> transport)
    : base_url_(TrimTrailingSlash(std::move(base_url))),
      transport_(std::move(transport)) {}

SdbdResult SdbdClient::Send(HttpRequest req) {
  SdbdResult out;
  if (!transport_) {
    out.error = "not_ready";
    return out;
  }
  if (!req.body.empty()) {
    const std::string sql_err = CheckRequestBody(req.body);
    if (!sql_err.empty()) {
      out.error = sql_err;
      out.status = 400;
      return out;
    }
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
    out.error = ErrorFromBody(out.body, res.status == 503 ? "not_ready"
                                                          : "HTTP " +
                                                                std::to_string(
                                                                    res.status));
  }
  return out;
}

SdbdResult SdbdClient::PostJson(const std::string& path, const std::string& body) {
  HttpRequest req;
  req.method = "POST";
  req.url = base_url_ + path;
  req.path_class = HttpRequest::PathClass::kQ;
  req.headers["Content-Type"] = "application/json";
  req.headers["Accept"] = "application/json";
  req.body = body;
  return Send(std::move(req));
}

SdbdResult SdbdClient::Health() {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/sdbd/api/v1/health";
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  SdbdResult r = Send(std::move(req));
  if (!r.ok && r.error.empty()) {
    r.error = "not_ready";
  }
  if (!r.ok) {
    if (r.error.find("HTTP") == 0) {
      r.error = "not_ready";
    }
  }
  return r;
}

SdbdResult SdbdClient::ListCatalog() {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/sdbd/api/v1/catalog";
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  SdbdResult r = Send(std::move(req));
  if (!r.ok) {
    return r;
  }
  std::vector<LayerInfo> layers;
  if (!ParseCatalog(r.body, &layers)) {
    r.ok = false;
    r.error = "bad_request";
  }
  return r;
}

SdbdResult SdbdClient::LayerInfoOf(const std::string& layer) {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + "/sdbd/api/v1/layers/" + layer;
  req.path_class = HttpRequest::PathClass::kMeta;
  req.headers["Accept"] = "application/json";
  return Send(std::move(req));
}

SdbdResult SdbdClient::QueryBBox(const OpenRequest& req) {
  return PostJson("/sdbd/api/v1/recordset/query", req.ToJson());
}

SdbdResult SdbdClient::Open(const OpenRequest& req) {
  return PostJson("/sdbd/api/v1/recordset/open", req.ToJson());
}

SdbdResult SdbdClient::Fetch(const std::string& handle, int offset, int limit) {
  std::ostringstream o;
  o << "{\"handle\":\"" << EscapeJsonString(handle) << "\",\"offset\":" << offset
    << ",\"limit\":" << limit << "}";
  return PostJson("/sdbd/api/v1/recordset/fetch", o.str());
}

SdbdResult SdbdClient::Close(const std::string& handle) {
  return PostJson("/sdbd/api/v1/recordset/close",
                  std::string("{\"handle\":\"") + EscapeJsonString(handle) +
                      "\"}");
}

bool SdbdClient::ParseLayerInfo(const std::string& json, LayerInfo* out) {
  json::Value v;
  if (!json::Parse(json, &v, nullptr)) {
    return false;
  }
  return FillLayerInfo(v, out);
}

bool SdbdClient::ParseCatalog(const std::string& json, std::vector<LayerInfo>* out) {
  if (!out) {
    return false;
  }
  out->clear();
  json::Value v;
  if (!json::Parse(json, &v, nullptr)) {
    return false;
  }
  if (v.is_object()) {
    if (v.String("type") == "FeatureCollection") {
      return false;
    }
    return false;
  }
  if (!v.is_array()) {
    return false;
  }
  for (const auto& item : v.array) {
    LayerInfo info;
    if (!FillLayerInfo(item, &info)) {
      return false;
    }
    out->push_back(std::move(info));
  }
  return true;
}

std::string SdbdClient::CheckRequestBody(const std::string& json) {
  if (json::HasForbiddenSqlKeys(json)) {
    return "forbidden_sql";
  }
  return {};
}

}  // namespace sg
