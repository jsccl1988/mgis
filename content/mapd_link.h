// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_MAPD_LINK_H
#define CONTENT_MAPD_LINK_H

#include "content/content_export.h"
#include "http_transport.h"

#include <memory>
#include <string>

namespace content {

struct MapdLink {
  std::string base_url;
  std::string layer;
};

CONTENT_EXPORT bool ParseMapdLinkUrl(const std::string& url, MapdLink* out);
CONTENT_EXPORT std::string DefaultMapdBaseUrl();
CONTENT_EXPORT bool MapdLinkRequested();
CONTENT_EXPORT bool EnvTruthy(const char* name);
CONTENT_EXPORT bool EnvSet(const char* name);

CONTENT_EXPORT bool TryOpenMapdLink(const std::string& url);
CONTENT_EXPORT bool TryOpenMapdLink(
    const std::string& url, std::shared_ptr<sg::HttpTransport> transport);

CONTENT_EXPORT std::string MapdLinkError();

}  // namespace content

#endif  // CONTENT_MAPD_LINK_H
