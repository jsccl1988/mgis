// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "third_party/maplibre/mview_map/camera_follow.h"

#include <cmath>

namespace mview_map {
namespace {

// WGS84 equatorial circumference, meters. Zoom 0 is one world wide.
// Each integer zoom stores half the previous height. Fractional zooms
// interpolate in log(height), which is the inverse of this table.
const double kTable[][2] = {
    {0.0, 40075016.685578488},  {1.0, 20037508.342789244},
    {2.0, 10018754.171394622},  {3.0, 5009377.085697311},
    {4.0, 2504688.5428486555},  {5.0, 1252344.2714243277},
    {6.0, 626172.13571216387},  {7.0, 313086.06785608194},
    {8.0, 156543.03392804097},  {9.0, 78271.516964020484},
    {10.0, 39135.758482010242}, {11.0, 19567.879241005121},
    {12.0, 9783.9396205025605}, {13.0, 4891.9698102512803},
    {14.0, 2445.9849051256401}, {15.0, 1222.9924525628201},
    {16.0, 611.49622628141003}, {17.0, 305.74811314070502},
    {18.0, 152.87405657035251}, {19.0, 76.437028285176254},
    {20.0, 38.218514142588127}, {21.0, 19.109257071294063},
    {22.0, 9.5546285356470317},
};

const int kTableSize = sizeof(kTable) / sizeof(kTable[0]);

int SegmentForZoom(double zoom) {
  if (zoom <= kTable[0][0])
    return 0;
  if (zoom >= kTable[kTableSize - 1][0])
    return kTableSize - 2;
  for (int i = 0; i < kTableSize - 1; ++i) {
    if (zoom >= kTable[i][0] && zoom <= kTable[i + 1][0])
      return i;
  }
  return kTableSize - 2;
}

int SegmentForHeight(double height_m) {
  if (height_m >= kTable[0][1])
    return 0;
  if (height_m <= kTable[kTableSize - 1][1])
    return kTableSize - 2;
  for (int i = 0; i < kTableSize - 1; ++i) {
    if (height_m <= kTable[i][1] && height_m >= kTable[i + 1][1])
      return i;
  }
  return kTableSize - 2;
}

double LerpLog(double h0, double h1, double t) {
  const double log0 = std::log(h0);
  const double log1 = std::log(h1);
  return std::exp(log0 + t * (log1 - log0));
}

}  // namespace

int ZoomTableSize() {
  return kTableSize;
}

double ZoomTableZoom(int index) {
  if (index < 0 || index >= kTableSize)
    return 0.0;
  return kTable[index][0];
}

double ZoomTableHeight(int index) {
  if (index < 0 || index >= kTableSize)
    return 0.0;
  return kTable[index][1];
}

void Follow2dMapToView(const MapCameraState& map, ViewCameraState* view) {
  if (!view)
    return;
  view->lon = map.lon;
  view->lat = map.lat;
  view->heading = map.bearing;
  view->pitch = map.pitch;

  const int seg = SegmentForZoom(map.zoom);
  const double z0 = kTable[seg][0];
  const double z1 = kTable[seg + 1][0];
  const double span = z1 - z0;
  double t = 0.0;
  if (span != 0.0)
    t = (map.zoom - z0) / span;
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;
  view->height_m = LerpLog(kTable[seg][1], kTable[seg + 1][1], t);
}

void Follow2dViewToMap(const ViewCameraState& view, MapCameraState* map) {
  if (!map)
    return;
  map->lon = view.lon;
  map->lat = view.lat;
  map->bearing = view.heading;
  map->pitch = view.pitch;

  const int seg = SegmentForHeight(view.height_m);
  const double h0 = kTable[seg][1];
  const double h1 = kTable[seg + 1][1];
  const double log0 = std::log(h0);
  const double log1 = std::log(h1);
  double t = 0.0;
  if (log1 != log0)
    t = (std::log(view.height_m) - log0) / (log1 - log0);
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;
  const double z0 = kTable[seg][0];
  const double z1 = kTable[seg + 1][0];
  map->zoom = z0 + t * (z1 - z0);
}

}  // namespace mview_map
