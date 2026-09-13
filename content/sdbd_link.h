// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// Entry for the mapd sibling's dispatcher. Do not call from OnPaint.
// HTTP is synchronous; invoke off the GUI thread.

#ifndef CONTENT_SDBD_LINK_H
#define CONTENT_SDBD_LINK_H

#include "content/content_export.h"
#include "core/sdb/http_transport.h"
#include "core/sdb/sdbd_types.h"

#include <memory>
#include <string>

class OGRLayer;
class GDALDataset;

namespace gfx2d {
class RenderDevice;
}

namespace content {

// Open sdbd:// or http(s):// (default SG_SDBD_BASE / http://127.0.0.1:8021).
// health fail → false, SdbdLinkError() == "not_ready". Never falls back to
// mapd or shapefile. Rejects :8020 (mapd port).
CONTENT_EXPORT std::string DefaultSdbdBaseUrl();
CONTENT_EXPORT bool TryOpenSdbdLink(const std::string& url);

// Test / injectable transport (FakeHttpTransport or WinHTTP).
CONTENT_EXPORT bool TryOpenSdbdLink(
    const std::string& url, std::shared_ptr<core::sdb::HttpTransport> transport);

CONTENT_EXPORT const char* SdbdLinkError();
CONTENT_EXPORT const char* SdbdLinkErrorMessage();
CONTENT_EXPORT OGRLayer* SdbdLinkLayer();
CONTENT_EXPORT GDALDataset* SdbdLinkDataset();
CONTENT_EXPORT const core::sdb::FeatureSet* SdbdLinkFeatures();
CONTENT_EXPORT const core::sdb::LayerInfo* SdbdLinkLayerInfo();

// Map Bind for M2. Sibling MapWindow should call this after TryOpenSdbdLink.
CONTENT_EXPORT bool BindSdbdLinkLayer(gfx2d::RenderDevice* device);

CONTENT_EXPORT void CloseSdbdLink();

}  // namespace content

#endif  // CONTENT_SDBD_LINK_H
