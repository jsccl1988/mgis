// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "core/sdb/sdbd_url.h"

#include "core/sdb/sdbd_types.h"
#include "spatial_link.h"

#include <cstdlib>
#include <string_view>

namespace core {
namespace sdb {
namespace {

struct legacy_sdbd_url_traits {
  static constexpr std::string_view alias_scheme = "sdbd";
  static constexpr std::string_view forbidden_port = "8020";
  static constexpr std::string_view forbidden_error =
      "this is a mapd port, use link mapd";
  static constexpr int default_port = 8021;
  static constexpr std::string_view extra_forbidden_ports[] = {"9030", "8005",
                                                              "8009"};
};

void fill_host_port(SdbdLinkUrl* out) {
  if (!out) {
    return;
  }
  auto se = out->base_url.find("://");
  std::string rest = (se == std::string::npos) ? out->base_url
                                               : out->base_url.substr(se + 3);
  if (!rest.empty() && rest.front() == '[') {
    const auto rb = rest.find(']');
    if (rb == std::string::npos) {
      return;
    }
    out->host = rest.substr(1, rb - 1);
    if (rb + 1 < rest.size() && rest[rb + 1] == ':') {
      out->port = std::atoi(rest.substr(rb + 2).c_str());
    }
    return;
  }
  const auto colon = rest.rfind(':');
  if (colon != std::string::npos) {
    out->host = rest.substr(0, colon);
    out->port = std::atoi(rest.substr(colon + 1).c_str());
  } else if (!rest.empty()) {
    out->host = rest;
  }
}

}  // namespace

std::string default_sdbd_base() {
  const char* env = std::getenv("SG_SDBD_BASE");
  if (env && *env) {
    std::string s = env;
    while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
      s.pop_back();
    }
    return s;
  }
  return kDefaultSdbdBase;
}

bool env_flag_set(const char* name) {
  const char* env = std::getenv(name);
  return env && env[0] == '1' && env[1] == '\0';
}

SdbdLinkUrl parse_sdbd_url(const std::string& raw) {
  SdbdLinkUrl out;
  const std::string url = raw.empty() ? default_sdbd_base() : raw;
  sg::SpatialLink link;
  if (!sg::parse_spatial_link<legacy_sdbd_url_traits>(url, &link)) {
    out.is_mapd_port = link.error.find("mapd") != std::string::npos;
    // core::sdb contract: non-mapd failures stay bad_request.
    out.error = out.is_mapd_port ? link.error : "bad_request";
    return out;
  }
  out.base_url = link.base_url;
  out.layer = link.layer;
  out.ok = true;
  fill_host_port(&out);
  return out;
}

}  // namespace sdb
}  // namespace core
