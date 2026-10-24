// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/view/camera_flight.h"

#include "gtest/gtest.h"

#include <cmath>

namespace {

TEST(CameraFlightTest, OrbitRoundTripKeepsEye) {
  cesium_flycube::OrbitController orbit;
  orbit.target[0] = 1000;
  orbit.target[1] = -250;
  orbit.target[2] = 40;
  orbit.yaw_radians = 0.7;
  orbit.pitch_radians = 0.45;
  orbit.distance = 3200;
  const cesium_flycube::CameraPose pose = cesium_flycube::PoseFromOrbit(orbit);
  const cesium_flycube::OrbitController back = cesium_flycube::OrbitFromPose(pose);
  EXPECT_NEAR(orbit.distance, back.distance, 1e-4);
  EXPECT_NEAR(orbit.yaw_radians, back.yaw_radians, 1e-4);
  EXPECT_NEAR(orbit.pitch_radians, back.pitch_radians, 1e-4);
  EXPECT_NEAR(pose.target[0], back.target[0], 1e-6);
}

TEST(CameraFlightTest, DragAndZoomChangeThePose) {
  cesium_flycube::OrbitController orbit;
  orbit.distance = 5000;
  const double yaw = orbit.yaw_radians;
  cesium_flycube::DragOrbit(&orbit, 40, 10);
  cesium_flycube::ZoomOrbit(&orbit, 2);
  EXPECT_GT(orbit.yaw_radians, yaw);
  EXPECT_LT(orbit.distance, 5000);
}

TEST(CameraFlightTest, ZoomAllowsSpaceScaleDistances) {
  cesium_flycube::OrbitController orbit;
  orbit.distance = 70000;
  orbit.max_distance = 4.0e7;
  for (int i = 0; i < 12; ++i) {
    cesium_flycube::ZoomOrbit(&orbit, -1);
  }
  EXPECT_GT(orbit.distance, 80000.0);
  EXPECT_LE(orbit.distance, orbit.max_distance);
  orbit.distance = 500;
  orbit.min_distance = 200;
  for (int i = 0; i < 20; ++i) {
    cesium_flycube::ZoomOrbit(&orbit, 1);
  }
  EXPECT_GE(orbit.distance, orbit.min_distance);
}

TEST(CameraFlightTest, FlightEndsOnTheDestination) {
  cesium_flycube::CameraPose from{};
  from.eye[0] = 0;
  from.eye[1] = -8000;
  from.eye[2] = 6000;
  from.up[2] = 1;
  cesium_flycube::CameraPose to{};
  to.eye[0] = 1200;
  to.eye[1] = -400;
  to.eye[2] = 900;
  to.target[0] = 500;
  to.up[2] = 1;
  cesium_flycube::Flight flight;
  cesium_flycube::BeginFlight(&flight, from, to, 4.0);
  flight.elapsed_seconds = 2.0;
  const cesium_flycube::CameraPose mid = cesium_flycube::SampleFlight(flight);
  EXPECT_GT(mid.eye[0], from.eye[0]);
  EXPECT_LT(mid.eye[0], to.eye[0]);
  flight.elapsed_seconds = 4.0;
  const cesium_flycube::CameraPose end = cesium_flycube::SampleFlight(flight);
  EXPECT_NEAR(end.eye[0], to.eye[0], 1e-6);
  EXPECT_NEAR(end.eye[2], to.eye[2], 1e-6);
  EXPECT_NEAR(end.target[0], to.target[0], 1e-6);
}

TEST(CameraFlightTest, GeodeticSampleReachesShanghaiApproach) {
  cesium_flycube::GeodeticView from;
  from.longitude_degrees = 121.20;
  from.latitude_degrees = 31.55;
  from.height_meters = 22000;
  from.heading_degrees = 40;
  from.pitch_degrees = -50;
  cesium_flycube::GeodeticView to;
  to.longitude_degrees = 121.50;
  to.latitude_degrees = 31.23;
  to.height_meters = 1600;
  to.heading_degrees = 15;
  to.pitch_degrees = -35;
  const cesium_flycube::GeodeticView end = cesium_flycube::SampleGeodetic(from, to, 1);
  EXPECT_NEAR(end.longitude_degrees, 121.50, 1e-9);
  EXPECT_NEAR(end.height_meters, 1600, 1e-6);
  const cesium_flycube::GeodeticView mid = cesium_flycube::SampleGeodetic(from, to, 0.5);
  EXPECT_GT(mid.height_meters, to.height_meters);
  EXPECT_LT(mid.height_meters, from.height_meters);
}

}  // namespace

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
