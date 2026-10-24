// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef VISTA_PASS_MAP_CAMERA_FOLLOW_H_
#define VISTA_PASS_MAP_CAMERA_FOLLOW_H_

namespace vista {

// MapLibre-style camera. zoom is the style zoom, not a height.
struct MapCameraState {
  double lon;
  double lat;
  double zoom;
  double bearing;
  double pitch;
};

// view0 camera in the follow-2d frame. heading matches bearing.
struct ViewCameraState {
  double lon;
  double lat;
  double height_m;
  double heading;
  double pitch;
};

// Fixed zoom-to-height knots. Index 0 is zoom 0.
int ZoomTableSize();
double ZoomTableZoom(int index);
double ZoomTableHeight(int index);

// follow-2d: map center and zoom become view0 lon/lat and height.
// The same table converts a view0 height back to zoom.
void Follow2dMapToView(const MapCameraState& map, ViewCameraState* view);
void Follow2dViewToMap(const ViewCameraState& view, MapCameraState* map);

}  // namespace vista

#endif  // VISTA_PASS_MAP_CAMERA_FOLLOW_H_
