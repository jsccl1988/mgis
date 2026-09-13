// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "core/sdb/sdbd_client.h"

#include "core/sdb/sdbd_json.h"

namespace core {
namespace sdb {
namespace {

std::string TrimSlash(std::string s) {
  while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
    s.pop_back();
  }
  return s;
}

void FillHttpError(SdbdResult* out) {
  if (!out) {
    return;
  }
  SdbdResult parsed;
  if (ParseErrorBody(out->body, &parsed) && !parsed.error.empty()) {
    out->error = parsed.error;
    out->message = parsed.message;
    return;
  }
  if (out->error.empty()) {
    out->error = (out->status == 503) ? "not_ready" : "bad_request";
  }
}

}  // namespace

SdbdClient::SdbdClient(std::string base_url,
                       std::shared_ptr<HttpTransport> transport)
    : base_url_(TrimSlash(std::move(base_url))),
      transport_(std::move(transport)) {}

SdbdResult SdbdClient::Send(HttpRequest req) {
  SdbdResult out;
  if (!transport_) {
    out.error = "not_ready";
    out.message = "null transport";
    return out;
  }
  if (BodyHasForbiddenSql(req.body)) {
    out.status = 400;
    out.error = "forbidden_sql";
    out.message = "sql keys are not allowed";
    return out;
  }
  HttpResponse res = transport_->Send(req);
  out.status = res.status;
  out.body = std::move(res.body);
  if (!res.error.empty()) {
    out.error = "not_ready";
    out.message = res.error;
    return out;
  }
  out.ok = res.status >= 200 && res.status < 300;
  if (!out.ok) {
    FillHttpError(&out);
    if (out.error.empty()) {
      out.error = (res.status == 503) ? "not_ready" : "bad_request";
    }
  }
  return out;
}

SdbdResult SdbdClient::Get(const std::string& path) {
  HttpRequest req;
  req.method = "GET";
  req.url = base_url_ + path;
  req.headers["Accept"] = "application/json";
  return Send(std::move(req));
}

SdbdResult SdbdClient::Post(const std::string& path, const std::string& body) {
  HttpRequest req;
  req.method = "POST";
  req.url = base_url_ + path;
  req.body = body;
  req.headers["Content-Type"] = "application/json";
  req.headers["Accept"] = "application/json";
  req.headers["Cache-Control"] = "no-store";
  return Send(std::move(req));
}

SdbdResult SdbdClient::Health() {
  SdbdResult out = Get(std::string(kApiPrefix) + "/health");
  if (!out.ok) {
    if (out.error.empty()) {
      out.error = "not_ready";
    }
    // Health failure is always not_ready — never fall back to mapd/shp.
    if (out.error != "not_ready") {
      out.message = out.error + (out.message.empty() ? "" : (": " + out.message));
      out.error = "not_ready";
    }
    return out;
  }
  Json json;
  if (!ParseJson(out.body, &json, nullptr) || !json.Bool("ok", false) ||
      json.String("service") != "sdbd") {
    out.ok = false;
    out.error = "not_ready";
    out.message = "health body is not sdbd";
  }
  return out;
}

std::vector<LayerInfo> SdbdClient::ListCatalog(std::string* err) {
  SdbdResult res = Get(std::string(kApiPrefix) + "/catalog");
  if (!res.ok) {
    if (err) {
      *err = res.error.empty() ? "bad_request" : res.error;
    }
    return {};
  }
  std::vector<LayerInfo> layers;
  std::string parse_err;
  if (!ParseCatalog(res.body, &layers, &parse_err)) {
    if (err) {
      *err = parse_err.empty() ? "bad_request" : parse_err;
    }
    return {};
  }
  return layers;
}

LayerInfo SdbdClient::LayerInfoOf(const std::string& layer, std::string* err) {
  SdbdResult res = Get(std::string(kApiPrefix) + "/layers/" + layer);
  if (!res.ok) {
    if (err) {
      *err = res.error.empty() ? "layer_not_found" : res.error;
    }
    return {};
  }
  LayerInfo info;
  std::string parse_err;
  if (!ParseLayerInfo(res.body, &info, &parse_err)) {
    if (err) {
      *err = parse_err.empty() ? "bad_request" : parse_err;
    }
    return {};
  }
  return info;
}

FeatureSet SdbdClient::QueryBBox(const OpenRequest& req, std::string* err) {
  SdbdResult res =
      Post(std::string(kApiPrefix) + "/recordset/query", OpenRequestToJson(req));
  if (!res.ok) {
    if (err) {
      *err = res.error.empty() ? "bad_request" : res.error;
    }
    return {};
  }
  FeatureSet set;
  std::string parse_err;
  if (!ParseFeatureSet(res.body, &set, &parse_err)) {
    if (err) {
      *err = parse_err.empty() ? "bad_request" : parse_err;
    }
    return {};
  }
  return set;
}

SdbdResult SdbdClient::RecordsetOpen(const OpenRequest& req) {
  return Post(std::string(kApiPrefix) + "/recordset/open",
              OpenRequestToJson(req));
}

FeatureSet SdbdClient::RecordsetFetch(const std::string& handle, int offset,
                                     int limit, std::string* err) {
  const std::string body = std::string("{\"handle\":\"") + JsonEscape(handle) +
                           "\",\"offset\":" + std::to_string(offset) +
                           ",\"limit\":" + std::to_string(limit) + "}";
  SdbdResult res = Post(std::string(kApiPrefix) + "/recordset/fetch", body);
  if (!res.ok) {
    if (err) {
      *err = res.error.empty() ? "handle_not_found" : res.error;
    }
    return {};
  }
  FeatureSet set;
  std::string parse_err;
  if (!ParseFeatureSet(res.body, &set, &parse_err)) {
    if (err) {
      *err = parse_err.empty() ? "bad_request" : parse_err;
    }
    return {};
  }
  return set;
}

SdbdResult SdbdClient::RecordsetClose(const std::string& handle) {
  const std::string body =
      std::string("{\"handle\":\"") + JsonEscape(handle) + "\"}";
  return Post(std::string(kApiPrefix) + "/recordset/close", body);
}

}  // namespace sdb
}  // namespace core
