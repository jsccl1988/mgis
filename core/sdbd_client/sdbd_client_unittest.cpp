// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// sdbd-M1 contract tests (04 §3 / link-spatial). Fake transport only unless
// SG_SDBD_REQUIRE=1 and :8021 is listening.

#include "sdbd_session.h"
#include "fake_http_transport.h"
#include "sdbd_client.h"
#include "sdbd_link.h"
#include "feature_set.h"
#include "testing/test.h"

#include <cstdlib>
#include <memory>
#include <string>

namespace {

const char kHealthOk[] = R"json({"ok":true,"service":"sdbd"})json";

const char kCatalogGolden[] = R"json([
  {
    "name": "t_rtree_demo",
    "crs": "EPSG:4326",
    "geom_type": "polygon",
    "geom_column": "geom",
    "extent": {"min_x": 0, "min_y": 0, "max_x": 1000, "max_y": 1000},
    "fields": [{"name": "id", "type": "uint64"}, {"name": "name", "type": "string"}]
  }
])json";

const char kNarrowFeatureSet[] = R"json({
  "layer": "t_rtree_demo",
  "crs": "EPSG:4326",
  "count": 1,
  "offset": 0,
  "features": [
    {
      "id": 2,
      "geom_wkt": "POLYGON((100 100,200 100,200 200,100 200,100 100))",
      "attrs": {"name": "hit"}
    }
  ]
})json";

const char kMapdPathQ[] = R"json({
  "type": "FeatureCollection",
  "features": [{"type": "Feature", "id": 1, "properties": {}, "geometry": null}]
})json";

bool HasForbiddenSqlKeys(const std::string& json) {
  return json.find("\"sql\"") != std::string::npos ||
         json.find("\"raw_sql\"") != std::string::npos ||
         json.find("\"raw_query\"") != std::string::npos;
}

std::shared_ptr<sg::FakeHttpTransport> MakeDemoTransport() {
  auto t = std::make_shared<sg::FakeHttpTransport>();
  t->set_handler("/sdbd/api/v1/health", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 200;
    r.body = kHealthOk;
    return r;
  });
  t->set_handler("/sdbd/api/v1/catalog", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 200;
    r.body = kCatalogGolden;
    return r;
  });
  t->set_handler("/sdbd/api/v1/layers/t_rtree_demo", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 200;
    r.body =
        R"({"name":"t_rtree_demo","crs":"EPSG:4326","geom_type":"polygon",)"
        R"("geom_column":"geom","extent":{"min_x":0,"min_y":0,"max_x":1000,"max_y":1000},)"
        R"("fields":[{"name":"id","type":"uint64"}]})";
    return r;
  });
  t->set_handler("/sdbd/api/v1/recordset/query", [](const sg::HttpRequest& req) {
    sg::HttpResponse r;
    if (HasForbiddenSqlKeys(req.body)) {
      r.status = 400;
      r.body = R"({"ok":false,"error":"forbidden_sql","message":"sql"})";
      return r;
    }
    r.status = 200;
    r.body = kNarrowFeatureSet;
    return r;
  });
  return t;
}

}  // namespace

TEST(SdbdLink, ParsesHttpAndSdbdScheme) {
  sg::SdbdLink link;
  ASSERT_TRUE(sg::ParseSdbdLink("http://127.0.0.1:8021", &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8021");
  EXPECT_TRUE(link.layer.empty());

  ASSERT_TRUE(sg::ParseSdbdLink("sdbd://127.0.0.1:8021/?layer=t_rtree_demo",
                                &link));
  EXPECT_EQ(link.base_url, "http://127.0.0.1:8021");
  EXPECT_EQ(link.layer, "t_rtree_demo");

  ASSERT_TRUE(sg::ParseSdbdLink("https://sdb.example.com:8021/", &link));
  EXPECT_EQ(link.base_url, "https://sdb.example.com:8021");
}

TEST(SdbdLink, RejectsMapdPort8020) {
  sg::SdbdLink link;
  ASSERT_FALSE(sg::ParseSdbdLink("http://127.0.0.1:8020", &link));
  EXPECT_NE(link.error.find("mapd"), std::string::npos);
}

TEST(SdbdClient, HealthAndCatalogFromFake) {
  auto t = MakeDemoTransport();
  sg::SdbdClient client("http://127.0.0.1:8021", t);

  const sg::SdbdResult health = client.Health();
  ASSERT_TRUE(health.ok);
  EXPECT_NE(health.body.find("sdbd"), std::string::npos);

  const sg::SdbdResult catalog = client.ListCatalog();
  ASSERT_TRUE(catalog.ok);
  std::vector<sg::LayerInfo> layers;
  ASSERT_TRUE(sg::SdbdClient::ParseCatalog(catalog.body, &layers));
  ASSERT_EQ(layers.size(), 1u);
  EXPECT_EQ(layers[0].name, "t_rtree_demo");
  EXPECT_EQ(layers[0].crs, "EPSG:4326");
  EXPECT_EQ(layers[0].extent.min_x, 0);
  EXPECT_EQ(layers[0].extent.max_x, 1000);
}

TEST(SdbdClient, ParseLayerInfoMatchesContract) {
  sg::LayerInfo info;
  ASSERT_TRUE(sg::SdbdClient::ParseLayerInfo(
      R"({"name":"t_rtree_demo","crs":"EPSG:4326","geom_type":"polygon",)"
      R"("geom_column":"geom","extent":{"min_x":0,"min_y":0,"max_x":1000,"max_y":1000},)"
      R"("fields":[{"name":"id","type":"uint64"},{"name":"name","type":"string"}]})",
      &info));
  EXPECT_EQ(info.name, "t_rtree_demo");
  EXPECT_EQ(info.geom_type, "polygon");
  ASSERT_EQ(info.fields.size(), 2u);
  EXPECT_EQ(info.fields[0].name, "id");
  EXPECT_EQ(info.fields[1].type, "string");
}

TEST(SdbdClient, QueryBBoxNarrowBoxId2) {
  auto t = MakeDemoTransport();
  sg::SdbdClient client("http://127.0.0.1:8021", t);

  sg::OpenRequest req;
  req.layer = "t_rtree_demo";
  req.columns = {"id", "name", "geom"};
  req.bbox = sg::BBox{100, 100, 200, 200};
  req.predicate = "intersects";
  req.crs = "EPSG:4326";
  req.limit = 10000;

  const std::string body = req.ToJson();
  EXPECT_FALSE(HasForbiddenSqlKeys(body));
  EXPECT_NE(body.find("\"bbox\":{"), std::string::npos);
  EXPECT_NE(body.find("\"min_x\""), std::string::npos);
  EXPECT_EQ(body.find("\"bbox\":["), std::string::npos);

  const sg::SdbdResult q = client.QueryBBox(req);
  ASSERT_TRUE(q.ok) << q.error;

  sg::feature_set::FeatureSet fs;
  ASSERT_TRUE(sg::feature_set::FeatureSet::FromSdbdJson(q.body, &fs));
  EXPECT_EQ(fs.count(), 1);
  EXPECT_EQ(fs.crs(), "EPSG:4326");
  ASSERT_TRUE(fs.MoveFirst());
  const sg::feature_set::Feature* f = fs.GetFeature();
  ASSERT_NE(f, nullptr);
  EXPECT_EQ(f->id, "2");
  EXPECT_FALSE(f->geom_wkt.empty());
  EXPECT_EQ(f->attrs.at("name"), "hit");
  EXPECT_FALSE(fs.MoveNext());
  EXPECT_TRUE(fs.IsEnd());
}

TEST(SdbdClient, HealthFailureIsNotReady) {
  auto t = std::make_shared<sg::FakeHttpTransport>();
  t->set_handler("/sdbd/api/v1/health", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 503;
    r.body = R"({"ok":false,"error":"not_ready"})";
    return r;
  });
  sg::SdbdClient client("http://127.0.0.1:8021", t);
  const sg::SdbdResult health = client.Health();
  EXPECT_FALSE(health.ok);
  EXPECT_EQ(health.error, "not_ready");
}

TEST(SdbdClient, PathQGeoJsonIsNotCatalog) {
  auto t = std::make_shared<sg::FakeHttpTransport>();
  t->set_handler("/sdbd/api/v1/catalog", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 200;
    r.body = kMapdPathQ;
    return r;
  });
  sg::SdbdClient client("http://127.0.0.1:8020", t);
  const sg::SdbdResult catalog = client.ListCatalog();
  EXPECT_FALSE(catalog.ok);
  std::vector<sg::LayerInfo> layers;
  EXPECT_FALSE(sg::SdbdClient::ParseCatalog(kMapdPathQ, &layers));
}

TEST(SdbdClient, QueryBodyWithSqlFails) {
  EXPECT_EQ(sg::SdbdClient::CheckRequestBody(
                R"({"layer":"t_rtree_demo","sql":"SELECT 1"})"),
            "forbidden_sql");
  EXPECT_EQ(sg::SdbdClient::CheckRequestBody(
                R"({"layer":"t_rtree_demo","raw_query":"x"})"),
            "forbidden_sql");
  sg::OpenRequest req;
  req.layer = "t_rtree_demo";
  EXPECT_EQ(sg::SdbdClient::CheckRequestBody(req.ToJson()), "");
}

TEST(SdbdSession, ConnectDoesNotFallback) {
  auto t = MakeDemoTransport();
  core::SdbdConnectOptions opt;
  opt.base_url = "http://127.0.0.1:8021";
  opt.layer = "t_rtree_demo";
  opt.open.bbox = sg::BBox{100, 100, 200, 200};
  core::SdbdConnectResult r = core::ConnectSdbd(opt, t);
  ASSERT_TRUE(r.ok);
  EXPECT_EQ(r.catalog.size(), 1u);
  EXPECT_EQ(r.features.count(), 1);
  ASSERT_TRUE(r.features.MoveFirst());
  EXPECT_EQ(r.features.GetFeature()->id, "2");
}

TEST(SdbdSession, HealthDownIsNotReadyNoShp) {
  auto t = std::make_shared<sg::FakeHttpTransport>();
  t->set_handler("/sdbd/api/v1/health", [](const sg::HttpRequest&) {
    sg::HttpResponse r;
    r.status = 503;
    r.body = R"({"ok":false,"error":"not_ready"})";
    return r;
  });
  core::SdbdConnectOptions opt;
  opt.base_url = "http://127.0.0.1:8021";
  const core::SdbdConnectResult r = core::ConnectSdbd(opt, t);
  EXPECT_FALSE(r.ok);
  EXPECT_EQ(r.error, "not_ready");
  EXPECT_EQ(r.features.count(), 0);
}

TEST(SdbdClientLive, SkipUnlessRequireAndListening) {
  if (std::getenv("SG_SDBD_SKIP")) {
    GTEST_SKIP() << "SG_SDBD_SKIP";
  }
  auto transport = sg::CreateDefaultHttpTransport();
  const char* base = std::getenv("SG_SDBD_BASE");
  const std::string url = base ? base : "http://127.0.0.1:8021";
  sg::SdbdClient client(
      url, std::shared_ptr<sg::HttpTransport>(std::move(transport)));
  const sg::SdbdResult health = client.Health();
  if (!health.ok) {
    if (std::getenv("SG_SDBD_REQUIRE")) {
      FAIL() << "sdbd not_ready: " << health.error;
    } else {
      GTEST_SKIP() << "sdbd not listening (expected until mogu P0): "
                   << health.error;
    }
  }
  EXPECT_NE(health.body.find("sdbd"), std::string::npos);
}
