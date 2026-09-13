// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// mapd-M1 contract tests (CONSUME_API / link-spatial). Fake transport only
// unless SG_MAPD_REQUIRE=1 and :8020 is listening.

#include "mapd_client.h"
#include "mapd_fake_transport.h"
#include "mapd_link.h"
#include "query_request.h"
#include "testing/test.h"

#include <cstdlib>
#include <memory>
#include <string>

namespace {

bool HasForbiddenSqlKeys(const std::string& json) {
  return json.find("\"sql\"") != std::string::npos ||
         json.find("\"raw_sql\"") != std::string::npos ||
         json.find("\"raw_query\"") != std::string::npos;
}

}  // namespace

TEST(MapdLink, ParsesHttpAndMapdScheme) {
  sg::MapdLink link;
  ASSERT_TRUE(sg::ParseMapdLink("http://127.0.0.1:8020", &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8020");

  ASSERT_TRUE(sg::ParseMapdLink("mapd://127.0.0.1:8020/?layer=basemap", &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8020");
  EXPECT_EQ(link.layer, "basemap");
}

TEST(MapdLink, RejectsSdbdPort8021) {
  sg::MapdLink link;
  ASSERT_FALSE(sg::ParseMapdLink("http://127.0.0.1:8021", &link));
  EXPECT_NE(link.error.find("sdbd"), std::string::npos);
}

TEST(QueryRequest, ToJsonIsWhitelistNoSql) {
  sg::QueryRequest req;
  req.layer = "basemap";
  req.bbox = {{116.3, 39.8, 116.5, 40.0}};
  req.limit = 100;
  const std::string body = req.ToJson();
  EXPECT_FALSE(HasForbiddenSqlKeys(body));
  EXPECT_NE(body.find("\"layer\":\"basemap\""), std::string::npos);
  EXPECT_NE(body.find("\"bbox\""), std::string::npos);
  EXPECT_NE(body.find("\"limit\":100"), std::string::npos);
  EXPECT_EQ(body.find("\"min_x\""), std::string::npos);
}

TEST(MapdClient, HealthCapabilitiesAndStyle) {
  auto t = sg::make_mapd_ready_transport<sg::mapd_client_fake_bind>();

  sg::MapdClient client("http://127.0.0.1:8020", t);
  const sg::MapdResult health = client.Health();
  ASSERT_TRUE(health.ok);

  const sg::MapdResult caps = client.GetCapabilities();
  ASSERT_TRUE(caps.ok);
  sg::Capabilities parsed;
  ASSERT_TRUE(sg::MapdClient::ParseCapabilities(caps.body, &parsed));
  EXPECT_EQ(parsed.crs, "EPSG:3857");
  EXPECT_EQ(parsed.data_version, "contract-2026-09-12.1");
  EXPECT_FALSE(parsed.postgis_configured);
  EXPECT_EQ(parsed.primary_store, "table");
  EXPECT_TRUE(parsed.path_q_features);
  ASSERT_FALSE(parsed.query_layers.empty());
  EXPECT_EQ(parsed.query_layers[0], "basemap");

  const sg::MapdResult style = client.GetStyle();
  ASSERT_TRUE(style.ok);

  sg::QueryRequest q;
  q.layer = "basemap";
  q.bbox = {{116.3, 39.8, 116.5, 40.0}};
  q.limit = 100;
  const sg::MapdResult qr = client.Query(q);
  ASSERT_TRUE(qr.ok);
}

TEST(MapdClient, HealthFailureIsNotReady) {
  auto t = std::make_shared<sg::FakeHttpTransport>();
  t->set_handler("/mapd/api/v1/health", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 503;
    r.body = R"({"ok":false})";
    return r;
  });
  sg::MapdClient client("http://127.0.0.1:8020", t);
  const sg::MapdResult health = client.Health();
  EXPECT_FALSE(health.ok);
  EXPECT_EQ(health.error, "not_ready");
}

TEST(MapdClient, TileUrlPathR) {
  sg::MapdClient client("http://127.0.0.1:8020", nullptr);
  EXPECT_EQ(client.TileUrl(sg::TileKind::kRaster, 3, 1, 2),
            "http://127.0.0.1:8020/mapd/tiles/raster/3/1/2.webp");
}

TEST(MapdClientLive, SkipUnlessRequireAndListening) {
  if (std::getenv("SG_MAPD_SKIP")) {
    GTEST_SKIP() << "SG_MAPD_SKIP";
  }
  auto transport = sg::CreateDefaultHttpTransport();
  const char* base = std::getenv("SG_MAPD_BASE");
  const std::string url = base ? base : "http://127.0.0.1:8020";
  sg::MapdClient client(
      url, std::shared_ptr<sg::HttpTransport>(std::move(transport)));
  const sg::MapdResult health = client.Health();
  if (!health.ok) {
    if (std::getenv("SG_MAPD_REQUIRE")) {
      FAIL() << "mapd not_ready: " << health.error;
    } else {
      GTEST_SKIP() << "mapd not listening: " << health.error;
    }
  }
}
