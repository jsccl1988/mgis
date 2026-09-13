// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <memory>
#include <string>

#include <ogrsf_frmts.h>

#include "core/sdb/ogr/gdal.hpp"
#include "core/sdb/fake_http_transport.h"
#include "core/sdb/http_transport.h"
#include "core/sdb/sdbd_client.h"
#include "core/sdb/sdbd_json.h"
#include "core/sdb/sdbd_mock_service.h"
#include "core/sdb/ogr_bind_traits.h"
#include "core/sdb/sdbd_ogr_mem.h"
#include "core/sdb/sdbd_url.h"
#include "testing/test.h"

#include "ogrsf_frmts.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <chrono>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#endif

namespace {

core::sdb::OpenRequest NarrowRequest() {
  core::sdb::OpenRequest req;
  req.layer = core::sdb::kDemoLayer;
  req.columns = {"id", "name", "geom"};
  req.bbox = {100, 100, 200, 200};
  req.predicate = "intersects";
  req.crs = core::sdb::kDemoCrs;
  req.limit = 10000;
  return req;
}

std::shared_ptr<core::sdb::FakeHttpTransport> ReadyTransport(
    core::sdb::SdbdMockService* mock) {
  auto transport = std::make_shared<core::sdb::FakeHttpTransport>();
  mock->Install(transport.get());
  return transport;
}

}  // namespace

TEST(SdbdUrl, ParsesSdbdSchemeAndLayer) {
  auto parsed = core::sdb::parse_sdbd_url(
      "sdbd://127.0.0.1:8021/?layer=t_rtree_demo");
  ASSERT_TRUE(parsed.ok);
  EXPECT_EQ(parsed.base_url, "http://127.0.0.1:8021");
  EXPECT_EQ(parsed.layer, "t_rtree_demo");
}

TEST(SdbdUrl, RejectsMapdPort) {
  auto parsed = core::sdb::parse_sdbd_url("http://127.0.0.1:8020");
  EXPECT_FALSE(parsed.ok);
  EXPECT_TRUE(parsed.is_mapd_port);
}

TEST(SdbdClient, HealthCatalogQueryNarrowGolden) {
  core::sdb::SdbdMockService mock;
  auto transport = ReadyTransport(&mock);
  core::sdb::SdbdClient client("http://127.0.0.1:8021", transport);

  auto health = client.Health();
  ASSERT_TRUE(health.ok) << health.message;
  EXPECT_NE(health.body.find("\"service\":\"sdbd\""), std::string::npos);

  std::string err;
  auto catalog = client.ListCatalog(&err);
  ASSERT_TRUE(err.empty()) << err;
  ASSERT_EQ(catalog.size(), 1u);
  EXPECT_EQ(catalog[0].name, "t_rtree_demo");
  EXPECT_EQ(catalog[0].crs, "EPSG:4326");

  err.clear();
  auto info = client.LayerInfoOf("t_rtree_demo", &err);
  ASSERT_TRUE(err.empty()) << err;
  EXPECT_EQ(info.geom_column, "geom");

  err.clear();
  auto set = client.QueryBBox(NarrowRequest(), &err);
  ASSERT_TRUE(err.empty()) << err;
  ASSERT_EQ(set.count, 1);
  ASSERT_EQ(set.features.size(), 1u);
  EXPECT_EQ(set.features[0].id, "2");
  set.MoveFirst();
  ASSERT_FALSE(set.IsEnd());
  ASSERT_NE(set.GetFeature(), nullptr);
  EXPECT_EQ(set.GetFeature()->id, "2");
}

TEST(SdbdClient, HealthFailIsNotReady) {
  core::sdb::SdbdMockService mock;
  mock.set_ready(false);
  auto transport = ReadyTransport(&mock);
  core::sdb::SdbdClient client("http://127.0.0.1:8021", transport);
  auto health = client.Health();
  EXPECT_FALSE(health.ok);
  EXPECT_EQ(health.error, "not_ready");
}

TEST(SdbdClient, ForbiddenSqlAndLimitAndPredicate) {
  core::sdb::SdbdMockService mock;
  // Drive the mock route directly for forbidden_sql / limit / predicate.
  int status = 0;
  std::string body;
  mock.Handle("POST", "/sdbd/api/v1/recordset/query",
              "{\"layer\":\"t_rtree_demo\",\"sql\":\"SELECT 1\"}", &status,
              &body);
  EXPECT_EQ(status, 400);
  EXPECT_NE(body.find("forbidden_sql"), std::string::npos);

  mock.Handle("POST", "/sdbd/api/v1/recordset/query",
              "{\"layer\":\"t_rtree_demo\",\"predicate\":\"contains\"}", &status,
              &body);
  EXPECT_EQ(status, 400);
  EXPECT_NE(body.find("unsupported_predicate"), std::string::npos);

  mock.Handle("POST", "/sdbd/api/v1/recordset/query",
              "{\"layer\":\"t_rtree_demo\",\"limit\":100001}", &status, &body);
  EXPECT_EQ(status, 400);
  EXPECT_NE(body.find("limit_exceeded"), std::string::npos);
}

TEST(OgrBind, FieldAndGeomTypeMaps) {
  EXPECT_EQ(core::sdb::ogr_bind_traits::field_type_of("int64"), OFTInteger64);
  EXPECT_EQ(core::sdb::ogr_bind_traits::field_type_of("double"), OFTReal);
  EXPECT_EQ(core::sdb::ogr_bind_traits::field_type_of("name"), OFTString);
  EXPECT_EQ(core::sdb::ogr_bind_traits::geom_type_of("Polygon"), wkbPolygon);
  EXPECT_EQ(core::sdb::ogr_bind_traits::geom_type_of("line"), wkbLineString);
  EXPECT_EQ(core::sdb::ogr_bind_traits::geom_type_of("point"), wkbPoint);
  EXPECT_EQ(core::sdb::ogr_bind_traits::geom_type_of(""), wkbUnknown);
  GDALDataset* ds = core::sdb::create_memory_dataset("ogr_bind_test");
  ASSERT_NE(ds, nullptr);
  GDALClose(ds);
}

TEST(SdbdOgrMem, WktFeaturesAreBindReady) {
  core::sdb::SdbdMockService mock;
  auto transport = ReadyTransport(&mock);
  core::sdb::SdbdClient client("http://127.0.0.1:8021", transport);
  std::string err;
  core::sdb::OpenRequest req;
  req.layer = core::sdb::kDemoLayer;
  req.bbox = {0, 0, 1000, 1000};
  req.predicate = "intersects";
  req.crs = core::sdb::kDemoCrs;
  auto set = client.QueryBBox(req, &err);
  ASSERT_TRUE(err.empty()) << err;
  ASSERT_EQ(set.GetFeatureCount(), 3u);

  core::sdb::LayerInfo schema = core::sdb::SdbdMockService::DemoLayerInfo();
  core::sdb::SdbdOgrMem mem;
  ASSERT_TRUE(mem.Load(set, &schema, &err)) << err;
  OGRLayer* layer = mem.layer();
  ASSERT_NE(layer, nullptr);
  EXPECT_EQ(layer->GetFeatureCount(), 3);
  layer->ResetReading();
  OGRFeature* feature = layer->GetNextFeature();
  ASSERT_NE(feature, nullptr);
  ASSERT_NE(feature->GetGeometryRef(), nullptr);
  OGRFeature::DestroyFeature(feature);
}

#if defined(_WIN32)
TEST(SdbdHttp, HangPeerTimesOut) {
  WSADATA wsa{};
  ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsa), 0);
  SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(listen_sock, INVALID_SOCKET);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;
  ASSERT_EQ(
      bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), 0);
  ASSERT_EQ(listen(listen_sock, 1), 0);
  int addr_len = sizeof(addr);
  ASSERT_EQ(getsockname(listen_sock, reinterpret_cast<sockaddr*>(&addr),
                        &addr_len),
            0);
  const int port = ntohs(addr.sin_port);

  std::atomic<bool> done{false};
  std::thread acceptor([&] {
    SOCKET client = accept(listen_sock, nullptr, nullptr);
    while (!done.load()) {
      Sleep(50);
    }
    if (client != INVALID_SOCKET) {
      closesocket(client);
    }
  });

  auto transport = core::sdb::CreateDefaultHttpTransport();
  core::sdb::HttpRequest req;
  req.method = "GET";
  req.url = "http://127.0.0.1:" + std::to_string(port) + "/sdbd/api/v1/health";
  const auto t0 = std::chrono::steady_clock::now();
  const core::sdb::HttpResponse res = transport->Send(req);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0)
                      .count();
  done.store(true);
  closesocket(listen_sock);
  acceptor.join();

  EXPECT_FALSE(res.error.empty()) << "status=" << res.status;
  EXPECT_LT(ms, 8000) << "WinHTTP hang ms=" << ms;
}
#endif
