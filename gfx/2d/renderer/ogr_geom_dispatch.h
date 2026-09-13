// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// OGR geometry-type → Draw* dispatch. Devices share this bind list
// instead of pasting a wkb* switch.

#pragma once

#include "base/error.h"
#include "ogrsf_frmts.h"

namespace gfx2d {
namespace detail {

template <typename Geom, OGRwkbGeometryType Wkb, auto Method>
struct ogr_draw_bind {
  using geom_type = Geom;
  static constexpr OGRwkbGeometryType wkb = Wkb;
  static constexpr auto method = Method;
};

// Fold over bind rows. No match → ERR_NONE (same as the old default:).
template <typename Device, typename... Binds>
int dispatch_render_geometry(Device& device, const OGRGeometry* geometry) {
  const OGRwkbGeometryType type = geometry->getGeometryType();
  int rc = ERR_NONE;
  ((type == Binds::wkb
        ? (rc = (device.*Binds::method)(
               static_cast<const typename Binds::geom_type*>(geometry)),
           true)
        : false) ||
   ...);
  return rc;
}

template <typename Device>
int dispatch_ogr_draw(Device& device, const OGRGeometry* geometry) {
  return dispatch_render_geometry<
      Device,
      ogr_draw_bind<OGRPoint, wkbPoint, &Device::DrawPoint>,
      ogr_draw_bind<OGRLineString, wkbLineString, &Device::DrawLineString>,
      ogr_draw_bind<OGRPolygon, wkbPolygon, &Device::DrawPolygon>,
      ogr_draw_bind<OGRMultiPoint, wkbMultiPoint, &Device::DrawMultiPoint>,
      ogr_draw_bind<OGRMultiLineString, wkbMultiLineString,
                    &Device::DrawMultiLineString>,
      ogr_draw_bind<OGRMultiPolygon, wkbMultiPolygon,
                    &Device::DrawMultiPolygon>,
      ogr_draw_bind<OGRLinearRing, wkbLinearRing, &Device::DrawLinearRing>>(
      device, geometry);
}

}  // namespace detail
}  // namespace gfx2d
