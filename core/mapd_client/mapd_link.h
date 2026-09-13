// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换. Parses http(s):// and mapd:// → http(s) base_url.
// Implementation is spatial_link_traits<kMapd>.

#pragma once

#include "core/sdbd_client/spatial_link.h"

namespace sg {

using MapdLink = SpatialLink;

inline bool ParseMapdLink(const std::string& input, MapdLink* out) {
  return parse_spatial_link<SpatialService::kMapd>(input, out);
}

}  // namespace sg
