// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "content/spatial_link.h"

#include "content/mapd_link.h"
#include "content/mapd_session.h"

#include <cstdlib>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if defined(MGIS_LINK_SDBD)
#include "content/sdbd_link.h"
#define MGIS_HAS_SDBD_LINK 1
#endif

namespace content {
namespace {

std::string g_kind;
std::string g_error;

bool StartsWith(const std::string& s, const char* prefix) {
  const std::size_t n = std::char_traits<char>::length(prefix);
  return s.size() >= n && s.compare(0, n, prefix) == 0;
}

bool Contains(const std::string& s, const char* needle) {
  return s.find(needle) != std::string::npos;
}

std::string FallbackSdbdBaseUrl() {
#if defined(MGIS_HAS_SDBD_LINK)
  return DefaultSdbdBaseUrl();
#else
  const char* e = std::getenv("SG_SDBD_BASE");
  if (e && *e) {
    return e;
  }
  return "http://127.0.0.1:8021";
#endif
}

bool CommandLineUrl(std::string* url_out) {
#if defined(_WIN32)
  const char* cmd = GetCommandLineA();
  if (!cmd) {
    return false;
  }
  const std::string line(cmd);
  const char* schemes[] = {"mapd://", "sdbd://", "http://", "https://"};
  for (const char* scheme : schemes) {
    const auto pos = line.find(scheme);
    if (pos == std::string::npos) {
      continue;
    }
    std::size_t end = pos;
    while (end < line.size() && line[end] != ' ' && line[end] != '"') {
      ++end;
    }
    if (url_out) {
      *url_out = line.substr(pos, end - pos);
    }
    return true;
  }
  const auto flag = line.find("--mapd=");
  if (flag != std::string::npos) {
    std::size_t start = flag + 7;
    if (start < line.size() && line[start] == '"') {
      ++start;
    }
    std::size_t end = start;
    while (end < line.size() && line[end] != ' ' && line[end] != '"') {
      ++end;
    }
    if (url_out) {
      *url_out = line.substr(start, end - start);
    }
    return !url_out->empty();
  }
  if (line.find("--mapd") != std::string::npos) {
    if (url_out) {
      *url_out = DefaultMapdBaseUrl();
    }
    return true;
  }
  if (line.find("--sdbd") != std::string::npos) {
    if (url_out) {
      *url_out = FallbackSdbdBaseUrl();
    }
    return true;
  }
#endif
  return false;
}

}  // namespace

bool LooksLikeMapdLink(const std::string& url) {
  if (url.empty()) {
    return false;
  }
  if (StartsWith(url, "mapd://")) {
    return true;
  }
  if (StartsWith(url, "sdbd://") || Contains(url, ":8021") ||
      Contains(url, "/sdbd/")) {
    return false;
  }
  return Contains(url, ":8020") || Contains(url, "/mapd/") ||
         StartsWith(url, "http://") || StartsWith(url, "https://");
}

bool LooksLikeSdbdLink(const std::string& url) {
  if (url.empty()) {
    return false;
  }
  if (StartsWith(url, "sdbd://")) {
    return true;
  }
  if (StartsWith(url, "mapd://") || Contains(url, ":8020") ||
      Contains(url, "/mapd/")) {
    return false;
  }
  return Contains(url, ":8021") || Contains(url, "/sdbd/");
}

bool SpatialLinkRequested(std::string* url_out) {
  std::string cmd;
  if (CommandLineUrl(&cmd)) {
    if (url_out) {
      *url_out = cmd;
    }
    return true;
  }
  if (MapdLinkRequested()) {
    if (url_out) {
      *url_out = DefaultMapdBaseUrl();
    }
    return true;
  }
  if (!EnvTruthy("SG_SDBD_SKIP") &&
      (EnvTruthy("SG_SDBD_LINK") || EnvTruthy("SG_SDBD_REQUIRE") ||
       EnvSet("SG_SDBD_BASE"))) {
    if (url_out) {
      *url_out = FallbackSdbdBaseUrl();
    }
    return true;
  }
  return false;
}

bool TryOpenSpatialLink(const std::string& url) {
  g_kind.clear();
  g_error.clear();

  if (LooksLikeSdbdLink(url)) {
    g_kind = "sdbd";
#if defined(MGIS_HAS_SDBD_LINK)
    const bool ok = TryOpenSdbdLink(url);
    if (!ok) {
      const char* err = SdbdLinkError();
      g_error = (err && *err) ? err : "not_ready";
    }
    return ok;
#else
    g_error = "sdbd_not_implemented";
    return false;
#endif
  }

  g_kind = "mapd";
  const bool ok = TryOpenMapdLink(url);
  if (!ok) {
    g_error = MapdLinkError();
    if (g_error.empty()) {
      g_error = "not_ready";
    }
  }
  return ok;
}

std::string SpatialLinkError() {
  if (!g_error.empty()) {
    return g_error;
  }
  if (g_kind == "mapd") {
    return MapdLinkError();
  }
#if defined(MGIS_HAS_SDBD_LINK)
  if (g_kind == "sdbd") {
    const char* err = SdbdLinkError();
    return (err && *err) ? err : std::string();
  }
#endif
  return {};
}

std::string SpatialLinkKind() { return g_kind; }

int SpatialLinkLayerCount() {
  if (g_kind == "mapd") {
    return SharedMapdSession().LayerCount();
  }
#if defined(MGIS_HAS_SDBD_LINK)
  if (g_kind == "sdbd") {
    if (GDALDataset* ds = SdbdLinkDataset()) {
      return ds->GetLayerCount();
    }
    return SdbdLinkLayer() ? 1 : 0;
  }
#endif
  return 0;
}

GDALDataset* TakeSpatialLinkDataset() {
  if (g_kind == "mapd") {
    return SharedMapdSession().ReleaseDataset();
  }
#if defined(MGIS_HAS_SDBD_LINK)
  if (g_kind == "sdbd") {
    return SdbdLinkDataset();
  }
#endif
  return nullptr;
}

}  // namespace content
