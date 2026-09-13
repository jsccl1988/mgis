// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "content/mapd_link.h"

#include "content/mapd_session.h"
#include "core/mapd_client/mapd_link.h"

#include <cstdlib>

namespace content {

bool EnvTruthy(const char* name) {
  const char* v = std::getenv(name);
  if (!v || !*v) {
    return false;
  }
  const std::string s(v);
  return s == "1" || s == "true" || s == "TRUE" || s == "yes";
}

bool EnvSet(const char* name) {
  const char* v = std::getenv(name);
  return v != nullptr && *v != '\0';
}

std::string DefaultMapdBaseUrl() {
  const char* e = std::getenv("SG_MAPD_BASE");
  if (e && *e) {
    sg::MapdLink parsed;
    if (sg::ParseMapdLink(e, &parsed)) {
      return parsed.base_url;
    }
    return e;
  }
  return "http://127.0.0.1:8020";
}

bool ParseMapdLinkUrl(const std::string& url, MapdLink* out) {
  if (!out) {
    return false;
  }
  *out = MapdLink{};
  sg::MapdLink parsed;
  if (!sg::ParseMapdLink(url, &parsed)) {
    return false;
  }
  out->base_url = parsed.base_url;
  out->layer = parsed.layer;
  return true;
}

bool MapdLinkRequested() {
  if (EnvTruthy("SG_MAPD_SKIP")) {
    return false;
  }
  if (EnvTruthy("SG_MAPD_LINK") || EnvTruthy("SG_MAPD_REQUIRE")) {
    return true;
  }
  if (EnvSet("SG_MAPD_BASE")) {
    return true;
  }
  return false;
}

bool TryOpenMapdLink(const std::string& url) {
  return SharedMapdSession().Open(url, nullptr);
}

bool TryOpenMapdLink(const std::string& url,
                     std::shared_ptr<sg::HttpTransport> transport) {
  return SharedMapdSession().Open(url, std::move(transport));
}

std::string MapdLinkError() { return SharedMapdSession().error(); }

}  // namespace content
