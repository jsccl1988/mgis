// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "gfx/2d/renderer/render_device.h"
#include "testing/test.h"

class TestRender2D : public ::testing::Test {
 protected:
  TestRender2D() {}
  ~TestRender2D() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestRender2D, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }

namespace {
using gfx2d::DRect;
using gfx2d::LPoint;
using gfx2d::LRect;
using gfx2d::Style;

class RhiCountingDevice : public gfx2d::RenderDevice {
 public:
  RhiCountingDevice() : RenderDevice(nullptr) {}
  int Init(HWND) override { return ERR_NONE; }
  int Destroy(void) override { return ERR_NONE; }
  int Release(void) override { return ERR_NONE; }
  int Resize(DRect) override { return ERR_NONE; }
  int Refresh(bool) override { return ERR_NONE; }
  int ZoomMove(LPoint) override { return ERR_NONE; }
  int ZoomScale(LPoint, float) override { return ERR_NONE; }
  int ZoomToRect(LRect) override { return ERR_NONE; }
  int LPToDP(float, float, long &, long &) const override { return ERR_NONE; }
  int DPToLP(long, long, float &, float &) const override { return ERR_NONE; }
  int LRectToDRect(const LRect &, DRect &) const override { return ERR_NONE; }
  int DRectToLRect(const DRect &, LRect &) const override { return ERR_NONE; }
  int Bind(const std::vector<OGRLayer *> &) override { return ERR_NONE; }
  int Unbind() override { return ERR_NONE; }
  int BeginRender(eRenderBuffer, bool, const Style *, int) override {
    return ERR_NONE;
  }
  int Render() override { return ERR_NONE; }
  int EndRender(eRenderBuffer) override { return ERR_NONE; }
  int Swap(void) override { return ERR_NONE; }
  int RenderLayer(const OGRLayer *, int) override { return ERR_NONE; }
  int RenderFeature(const OGRFeature *, int) override { return ERR_NONE; }
  int RenderGeometry(const OGRGeometry *, int) override { return ERR_NONE; }
  int DrawMultiLineString(const OGRMultiLineString *) override {
    return ERR_NONE;
  }
  int DrawMultiPoint(const OGRMultiPoint *) override { return ERR_NONE; }
  int DrawMultiPolygon(const OGRMultiPolygon *) override { return ERR_NONE; }
  int DrawPoint(const OGRPoint *) override { return ERR_NONE; }
  int DrawAnno(const OGRPoint *, const char *, float, float, float,
               float) override {
    return ERR_NONE;
  }
  int DrawSymbol(const OGRPoint *, HICON, long, long) override {
    return ERR_NONE;
  }
  int DrawLineString(const OGRLineString *) override { return ERR_NONE; }
  int DrawLinearRing(const OGRLinearRing *) override { return ERR_NONE; }
  int DrawPolygon(const OGRPolygon *) override { return ERR_NONE; }
  int DrawImage(const char *, int, const LRect &, long,
                eRenderBuffer) override {
    return ERR_NONE;
  }
  int StrethImage(const char *, int, const LRect &, long,
                  eRenderBuffer) override {
    return ERR_NONE;
  }
  int SaveImage(const char *, eRenderBuffer, bool) override { return ERR_NONE; }
  int Save2ImageBuffer(char *&, long &, long, eRenderBuffer, bool) override {
    return ERR_NONE;
  }
  int FreeImageBuffer(char *&) override { return ERR_NONE; }
};
}  // namespace

TEST_F(TestRender2D, encoder_submit) {
  RhiCountingDevice device;
  device.Begin(gfx2d::RenderDevice::RB_MAP, 0.18f, 0.10f, 0.08f, 1.f);
  device.SetPipeline("fill_poly");
  device.Draw(3);
  device.DrawIndexed(6);
  device.End();
  EXPECT_TRUE(device.closed());
  device.Submit();
  EXPECT_EQ(1ull, device.submit_count());
  EXPECT_EQ(1ull, device.draw_count());
  EXPECT_EQ(1ull, device.draw_indexed_count());
}