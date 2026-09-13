// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/mapd_link.h"
#include "content/mapd_session.h"
#include "content/spatial_link.h"
#include "mapd_fake_transport.h"

#include <memory>
#include <string>

#include "ogrsf_frmts.h"
#include "testing/test.h"

TEST(MapdLink, ParseMapdSchemeAndLayer) {
  content::MapdLink link;
  ASSERT_TRUE(content::ParseMapdLinkUrl("mapd://127.0.0.1:8020", &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8020");
  EXPECT_TRUE(link.layer.empty());

  ASSERT_TRUE(
      content::ParseMapdLinkUrl("mapd://127.0.0.1:8020/?layer=basemap", &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8020");
  EXPECT_EQ(link.layer, "basemap");

  ASSERT_TRUE(content::ParseMapdLinkUrl("https://map.example.com:8020", &link));
  EXPECT_EQ(link.base_url, "https://map.example.com:8020");

  EXPECT_FALSE(content::ParseMapdLinkUrl("sgdata://mapd/fnrpc/127.0.0.1:8020",
                                         &link));
}

TEST(MapdLink, DefaultBase) {
  const std::string base = content::DefaultMapdBaseUrl();
  EXPECT_FALSE(base.empty());
  if (!content::EnvSet("SG_MAPD_BASE")) {
    EXPECT_EQ(base, "http://127.0.0.1:8020");
  }
}

TEST(MapdLink, FakeOpenBindsGeoJsonMem) {
  GDALAllRegister();
  auto transport = sg::make_mapd_ready_transport<sg::mapd_link_fake_bind>();
  ASSERT_TRUE(content::TryOpenMapdLink("http://127.0.0.1:8020", transport));
  EXPECT_EQ(content::MapdLinkError(), "");
  ASSERT_GT(content::SharedMapdSession().LayerCount(), 0);
  GDALDataset* ds = content::SharedMapdSession().dataset();
  ASSERT_NE(ds, nullptr);
  OGRLayer* layer = ds->GetLayer(0);
  ASSERT_NE(layer, nullptr);
  EXPECT_GT(layer->GetFeatureCount(), 0);
}

TEST(MapdLink, HealthFailIsNotReadyNoDataset) {
  GDALAllRegister();
  auto transport = std::make_shared<sg::FakeHttpTransport>();
  transport->set_handler("/mapd/api/v1/health", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 503;
    r.body = R"({"error":"not_ready"})";
    return r;
  });
  EXPECT_FALSE(content::TryOpenMapdLink("http://127.0.0.1:8020", transport));
  EXPECT_EQ(content::MapdLinkError(), "not_ready");
  EXPECT_EQ(content::SharedMapdSession().dataset(), nullptr);
}

TEST(SpatialLink, DispatchMapdVsSdbd) {
  EXPECT_TRUE(content::LooksLikeMapdLink("mapd://127.0.0.1:8020"));
  EXPECT_TRUE(content::LooksLikeMapdLink("http://127.0.0.1:8020"));
  EXPECT_FALSE(content::LooksLikeMapdLink("sdbd://127.0.0.1:8021"));
  EXPECT_TRUE(content::LooksLikeSdbdLink("sdbd://127.0.0.1:8021"));
  EXPECT_TRUE(content::LooksLikeSdbdLink("http://127.0.0.1:8021"));
  EXPECT_FALSE(content::LooksLikeSdbdLink("http://127.0.0.1:8020"));

  EXPECT_FALSE(content::TryOpenSpatialLink("sdbd://127.0.0.1:8021"));
  EXPECT_EQ(content::SpatialLinkKind(), "sdbd");
  EXPECT_FALSE(content::SpatialLinkError().empty());
  EXPECT_EQ(content::SpatialLinkLayerCount(), 0);
  EXPECT_EQ(content::TakeSpatialLinkDataset(), nullptr);
}
