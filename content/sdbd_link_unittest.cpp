// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include <memory>
#include <string>

#include "content/sdbd_link.h"
#include "content/spatial_link.h"
#include "core/sdb/fake_http_transport.h"
#include "core/sdb/sdbd_mock_service.h"
#include "testing/sdbd/sdbd_http_mock.h"
#include "testing/test.h"

#include "ogrsf_frmts.h"

TEST(SdbdLink, TryOpenAgainstFakeThenBindReady) {
  core::sdb::SdbdMockService mock;
  auto transport = std::make_shared<core::sdb::FakeHttpTransport>();
  mock.Install(transport.get());

  ASSERT_TRUE(content::TryOpenSdbdLink("sdbd://127.0.0.1:8021/?layer=t_rtree_demo",
                                      transport));
  EXPECT_STREQ(content::SdbdLinkError(), "");
  ASSERT_NE(content::SdbdLinkLayer(), nullptr);
  EXPECT_EQ(content::SdbdLinkLayer()->GetFeatureCount(), 3);
  ASSERT_NE(content::SdbdLinkFeatures(), nullptr);
  EXPECT_EQ(content::SdbdLinkFeatures()->GetFeatureCount(), 3u);

  content::SdbdLinkLayer()->ResetReading();
  OGRFeature* feature = content::SdbdLinkLayer()->GetNextFeature();
  ASSERT_NE(feature, nullptr);
  ASSERT_NE(feature->GetGeometryRef(), nullptr);
  OGRFeature::DestroyFeature(feature);

  EXPECT_FALSE(content::BindSdbdLinkLayer(nullptr));
  content::CloseSdbdLink();
}

TEST(SdbdLink, HealthFailIsNotReadyNoFallback) {
  content::CloseSdbdLink();
  core::sdb::SdbdMockService mock;
  mock.set_ready(false);
  auto transport = std::make_shared<core::sdb::FakeHttpTransport>();
  mock.Install(transport.get());

  EXPECT_FALSE(content::TryOpenSdbdLink("http://127.0.0.1:8021", transport));
  EXPECT_STREQ(content::SdbdLinkError(), "not_ready");
  EXPECT_EQ(content::SdbdLinkLayer(), nullptr);
  content::CloseSdbdLink();
}

TEST(SdbdLink, RejectsMapdPort) {
  EXPECT_FALSE(content::TryOpenSdbdLink("http://127.0.0.1:8020"));
  EXPECT_TRUE(std::string(content::SdbdLinkErrorMessage())
                  .find("mapd") != std::string::npos);
  content::CloseSdbdLink();
}

TEST(SdbdLink, LiveHttpMockHealthCatalogQueryBind) {
  testing::sdbd::SdbdHttpMock server;
  ASSERT_TRUE(server.StartAny()) << "failed to bind mock listener";
  ASSERT_NE(server.port(), 8020) << "ephemeral port collided with mapd";
  const std::string url = server.base_url() + "/?layer=t_rtree_demo";
  ASSERT_TRUE(content::TryOpenSdbdLink(url)) << content::SdbdLinkErrorMessage();
  ASSERT_NE(content::SdbdLinkLayer(), nullptr);
  EXPECT_GE(content::SdbdLinkLayer()->GetFeatureCount(), 1);
  content::SdbdLinkLayer()->ResetReading();
  OGRFeature* feature = content::SdbdLinkLayer()->GetNextFeature();
  ASSERT_NE(feature, nullptr);
  ASSERT_NE(feature->GetGeometryRef(), nullptr);
  OGRFeature::DestroyFeature(feature);
  content::CloseSdbdLink();
}

TEST(SpatialLink, SdbdOpenExposesLayerCountAndDataset) {
  testing::sdbd::SdbdHttpMock server;
  ASSERT_TRUE(server.StartAny()) << "failed to bind mock listener";
  ASSERT_NE(server.port(), 8020) << "ephemeral port collided with mapd";
  const std::string url = std::string("sdbd://127.0.0.1:") +
                          std::to_string(server.port()) + "/?layer=t_rtree_demo";
  ASSERT_TRUE(content::LooksLikeSdbdLink(url));
  ASSERT_TRUE(content::TryOpenSpatialLink(url)) << content::SpatialLinkError();
  EXPECT_EQ(content::SpatialLinkKind(), "sdbd");
  EXPECT_GE(content::SpatialLinkLayerCount(), 1);
  EXPECT_EQ(content::TakeSpatialLinkDataset(), content::SdbdLinkDataset());
  ASSERT_NE(content::SdbdLinkLayer(), nullptr);
  content::CloseSdbdLink();
}
