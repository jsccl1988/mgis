// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_SPATIAL_LINK_H
#define CONTENT_SPATIAL_LINK_H

#include "content/content_export.h"

#include <string>

#include "ogrsf_frmts.h"

namespace content {

// Menu commands: 「链接 mapd」 / 「链接 sdbd」
#ifndef MESSAGE_CMD_LINK_MAPD
#define MESSAGE_CMD_LINK_MAPD 0x4001
#endif
#ifndef MESSAGE_CMD_LINK_SDBD
#define MESSAGE_CMD_LINK_SDBD 0x4002
#endif

CONTENT_EXPORT bool LooksLikeMapdLink(const std::string& url);
CONTENT_EXPORT bool LooksLikeSdbdLink(const std::string& url);

CONTENT_EXPORT bool SpatialLinkRequested(std::string* url_out);
CONTENT_EXPORT bool TryOpenSpatialLink(const std::string& url);

CONTENT_EXPORT std::string SpatialLinkError();
CONTENT_EXPORT std::string SpatialLinkKind();
CONTENT_EXPORT int SpatialLinkLayerCount();
CONTENT_EXPORT GDALDataset* TakeSpatialLinkDataset();

}  // namespace content

#endif  // CONTENT_SPATIAL_LINK_H
