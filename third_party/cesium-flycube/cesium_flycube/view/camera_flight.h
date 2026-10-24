// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <algorithm>
#include <cmath>

namespace cesium_flycube {

// Shared orbit and flight samples for the FlyCube window and the Unreal pawn.
// Positions are meters. Geodetic fields are degrees and meters above the ellipsoid.
struct CameraPose {
  double eye[3];
  double target[3];
  double up[3];
};

struct OrbitController {
  double target[3] = {0, 0, 0};
  double yaw_radians = 0.8;
  double pitch_radians = 0.55;
  double distance = 4000;
  // Google Earth-style space zoom needs tens of thousands of km; city browse
  // stays near the defaults.
  double min_distance = 80.0;
  double max_distance = 4.0e7;
};

struct Flight {
  CameraPose from{};
  CameraPose to{};
  double duration_seconds = 1;
  double elapsed_seconds = 0;
  bool active = false;
};

struct GeodeticView {
  double longitude_degrees = 0;
  double latitude_degrees = 0;
  double height_meters = 1000;
  double heading_degrees = 0;
  double pitch_degrees = -35;
};

inline double ClampDouble(double value, double lo, double hi) {
  // Parentheses keep Windows min/max macros from eating the call.
  return (std::max)(lo, (std::min)(hi, value));
}

inline double Smooth01(double t) {
  t = ClampDouble(t, 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

inline CameraPose PoseFromOrbit(const OrbitController& orbit) {
  const double cp = std::cos(orbit.pitch_radians);
  const double sp = std::sin(orbit.pitch_radians);
  const double cy = std::cos(orbit.yaw_radians);
  const double sy = std::sin(orbit.yaw_radians);
  CameraPose pose;
  pose.eye[0] = orbit.target[0] + orbit.distance * cp * cy;
  pose.eye[1] = orbit.target[1] + orbit.distance * cp * sy;
  pose.eye[2] = orbit.target[2] + orbit.distance * sp;
  pose.target[0] = orbit.target[0];
  pose.target[1] = orbit.target[1];
  pose.target[2] = orbit.target[2];
  pose.up[0] = 0;
  pose.up[1] = 0;
  pose.up[2] = 1;
  return pose;
}

inline OrbitController OrbitFromPose(const CameraPose& pose) {
  OrbitController orbit;
  orbit.target[0] = pose.target[0];
  orbit.target[1] = pose.target[1];
  orbit.target[2] = pose.target[2];
  const double dx = pose.eye[0] - pose.target[0];
  const double dy = pose.eye[1] - pose.target[1];
  const double dz = pose.eye[2] - pose.target[2];
  orbit.distance = std::sqrt(dx * dx + dy * dy + dz * dz);
  if (orbit.distance < 1.0) {
    orbit.distance = 1.0;
  }
  orbit.yaw_radians = std::atan2(dy, dx);
  orbit.pitch_radians =
      std::asin(ClampDouble(dz / orbit.distance, -1.0, 1.0));
  return orbit;
}

inline void DragOrbit(OrbitController* orbit, double dx_pixels, double dy_pixels) {
  orbit->yaw_radians += dx_pixels * 0.005;
  // Wider pitch than city browse so a space view can tilt toward the horizon.
  orbit->pitch_radians =
      ClampDouble(orbit->pitch_radians - dy_pixels * 0.004, 0.05, 1.45);
}

inline void ZoomOrbit(OrbitController* orbit, double wheel_steps) {
  orbit->distance = ClampDouble(orbit->distance * std::pow(0.9, wheel_steps),
                                orbit->min_distance, orbit->max_distance);
}

inline void PanOrbit(OrbitController* orbit, double dx_pixels, double dy_pixels) {
  const CameraPose pose = PoseFromOrbit(*orbit);
  double forward[3] = {pose.target[0] - pose.eye[0], pose.target[1] - pose.eye[1],
                       pose.target[2] - pose.eye[2]};
  const double length =
      std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] +
                forward[2] * forward[2]);
  if (length < 1e-8) {
    return;
  }
  forward[0] /= length;
  forward[1] /= length;
  forward[2] /= length;
  const double up[3] = {0, 0, 1};
  double right[3] = {forward[1] * up[2] - forward[2] * up[1],
                     forward[2] * up[0] - forward[0] * up[2],
                     forward[0] * up[1] - forward[1] * up[0]};
  const double right_length = std::sqrt(right[0] * right[0] + right[1] * right[1] +
                                        right[2] * right[2]);
  if (right_length < 1e-8) {
    return;
  }
  right[0] /= right_length;
  right[1] /= right_length;
  right[2] /= right_length;
  const double cam_up[3] = {right[1] * forward[2] - right[2] * forward[1],
                            right[2] * forward[0] - right[0] * forward[2],
                            right[0] * forward[1] - right[1] * forward[0]};
  const double scale = orbit->distance * 0.0015;
  orbit->target[0] += (-right[0] * dx_pixels + cam_up[0] * dy_pixels) * scale;
  orbit->target[1] += (-right[1] * dx_pixels + cam_up[1] * dy_pixels) * scale;
  orbit->target[2] += (-right[2] * dx_pixels + cam_up[2] * dy_pixels) * scale;
}

inline void BeginFlight(Flight* flight,
                        const CameraPose& from,
                        const CameraPose& to,
                        double seconds) {
  flight->from = from;
  flight->to = to;
  flight->duration_seconds = seconds <= 0.0 ? 0.001 : seconds;
  flight->elapsed_seconds = 0;
  flight->active = true;
}

inline CameraPose SampleFlight(const Flight& flight) {
  const double u = Smooth01(flight.elapsed_seconds / flight.duration_seconds);
  if (u <= 0.0) {
    return flight.from;
  }
  if (u >= 1.0) {
    return flight.to;
  }
  CameraPose pose;
  for (int i = 0; i < 3; ++i) {
    pose.eye[i] = flight.from.eye[i] + (flight.to.eye[i] - flight.from.eye[i]) * u;
    pose.target[i] =
        flight.from.target[i] + (flight.to.target[i] - flight.from.target[i]) * u;
    pose.up[i] = flight.from.up[i] + (flight.to.up[i] - flight.from.up[i]) * u;
  }
  return pose;
}

inline GeodeticView SampleGeodetic(const GeodeticView& from,
                                   const GeodeticView& to,
                                   double t) {
  const double u = Smooth01(t);
  GeodeticView view;
  view.longitude_degrees =
      from.longitude_degrees + (to.longitude_degrees - from.longitude_degrees) * u;
  view.latitude_degrees =
      from.latitude_degrees + (to.latitude_degrees - from.latitude_degrees) * u;
  view.height_meters = from.height_meters + (to.height_meters - from.height_meters) * u;
  view.heading_degrees =
      from.heading_degrees + (to.heading_degrees - from.heading_degrees) * u;
  view.pitch_degrees = from.pitch_degrees + (to.pitch_degrees - from.pitch_degrees) * u;
  return view;
}

inline void DragGeodetic(GeodeticView* view, double dx_pixels, double dy_pixels) {
  view->heading_degrees += dx_pixels * 0.12;
  view->pitch_degrees = ClampDouble(view->pitch_degrees - dy_pixels * 0.08, -85.0, -8.0);
}

inline void ZoomGeodetic(GeodeticView* view, double wheel_steps) {
  view->height_meters =
      ClampDouble(view->height_meters * std::pow(0.9, wheel_steps), 80.0, 2.0e7);
}

}  // namespace cesium_flycube
