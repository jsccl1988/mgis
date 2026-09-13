// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换. Parses http(s):// and sdbd:// → http(s) base_url + optional layer.
// Implementation is spatial_link_traits<kSdbd>.

#pragma once

#include "core/sdbd_client/spatial_link.h"

namespace sg {

using SdbdLink = SpatialLink;

inline bool ParseSdbdLink(const std::string& input, SdbdLink* out) {
  return parse_spatial_link<SpatialService::kSdbd>(input, out);
}

}  // namespace sg
