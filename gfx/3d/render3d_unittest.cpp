// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "gfx/3d/renderer/render_device.h"
#include "testing/test.h"

class TestRender3D : public ::testing::Test {
 protected:
  TestRender3D() {}
  ~TestRender3D() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestRender3D, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }

TEST_F(TestRender3D, encoder_submit) {
  gfx3d::RenderDevice device(nullptr);
  const float xyz[] = {0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
  const unsigned idx[] = {0, 1, 2};
  device.Begin(0.1f, 0.2f, 0.3f, 1.f);
  device.SetPipeline("fill_poly");
  device.SetMesh(xyz, 3, idx, 3);
  device.DrawIndexed(3);
  device.End();
  EXPECT_TRUE(device.closed());
  device.Submit();
  EXPECT_EQ(1ull, device.submit_count());
}