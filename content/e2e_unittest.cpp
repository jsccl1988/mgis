// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Automated e2e for default world InitMap data, public spatial-link APIs,
// and a brief smartgis.exe stay-up check. Re-run:
//   out\content_test.exe --gtest_filter=E2E.*
//   python testing/e2e/smartgis_smoke.py

#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "base/path/base_paths.h"
#include "base/util/string_util.h"
#include "content/mapd_link.h"
#include "content/mapd_session.h"
#include "content/sdbd_link.h"
#include "content/spatial_link.h"
#include "core/sdb/ogr_bind_traits.h"
#include "mapd_fake_transport.h"
#include "ogrsf_frmts.h"
#include "testing/sdbd/sdbd_http_mock.h"
#include "testing/test.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace {

const char kWorldFiles[][24] = {"countries.geojson", "graticule.geojson",
                                "capitals.geojson"};
const OGRwkbGeometryType kWorldGeom[] = {wkbPolygon, wkbLineString, wkbPoint};
const char* kWorldNames[] = {"countries", "graticule", "capitals"};

bool EnvTruthy(const char* name) { return content::EnvTruthy(name); }

std::string JoinPath(const std::string& dir, const char* file) {
  if (dir.empty()) {
    return file;
  }
  if (dir.back() == '\\' || dir.back() == '/') {
    return dir + file;
  }
  return dir + "\\" + file;
}

std::string FindWorldDir() {
  const char* rel[] = {"data\\world", "..\\core\\data\\world"};
  base::PathString exe;
  if (base::PathProvider(base::DIR_EXE, &exe)) {
    const std::string root = base::UTF16ToUTF8(exe);
    for (const char* suffix : rel) {
      const std::string dir = JoinPath(root, suffix);
      if (base::PathExists(base::UTF8ToUTF16(dir + "\\countries.geojson"))) {
        return dir;
      }
    }
  }
  base::PathString cwd;
  if (base::GetCurrentDirectory(&cwd)) {
    const std::string root = base::UTF16ToUTF8(cwd);
    for (const char* suffix : rel) {
      const std::string dir = JoinPath(root, suffix);
      if (base::PathExists(base::UTF8ToUTF16(dir + "\\countries.geojson"))) {
        return dir;
      }
    }
  }
  return {};
}

bool FileExistsUtf8(const std::string& path) {
  return base::PathExists(base::UTF8ToUTF16(path));
}

}  // namespace

TEST(E2E, WorldDemoBindsPolygonLineStringPoint) {
  GDALAllRegister();
  const std::string dir = FindWorldDir();
  ASSERT_FALSE(dir.empty()) << "world geojson not found next to exe or cwd";

  GDALDataset* ds = core::sdb::create_memory_dataset("world_demo_e2e");
  ASSERT_NE(ds, nullptr);

  int copied = 0;
  for (int i = 0; i < 3; ++i) {
    const std::string path = JoinPath(dir, kWorldFiles[i]);
    ASSERT_TRUE(FileExistsUtf8(path)) << path;
    GDALDataset* src = static_cast<GDALDataset*>(GDALOpenEx(
        path.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, nullptr,
        nullptr));
    ASSERT_NE(src, nullptr) << path << " " << CPLGetLastErrorMsg();
    OGRLayer* layer = src->GetLayer(0);
    ASSERT_NE(layer, nullptr);
    const OGRwkbGeometryType geom =
        wkbFlatten(layer->GetGeomType());
    EXPECT_EQ(geom, kWorldGeom[i]) << kWorldFiles[i];
    EXPECT_GT(layer->GetFeatureCount(), 0);
    ASSERT_NE(ds->CopyLayer(layer, kWorldNames[i]), nullptr);
    ++copied;
    GDALClose(src);
  }
  EXPECT_EQ(copied, 3);
  ASSERT_EQ(ds->GetLayerCount(), 3);
  EXPECT_STREQ(ds->GetLayer(0)->GetName(), "countries");
  EXPECT_EQ(wkbFlatten(ds->GetLayer(0)->GetGeomType()), wkbPolygon);
  EXPECT_STREQ(ds->GetLayer(1)->GetName(), "graticule");
  EXPECT_EQ(wkbFlatten(ds->GetLayer(1)->GetGeomType()), wkbLineString);
  EXPECT_STREQ(ds->GetLayer(2)->GetName(), "capitals");
  EXPECT_EQ(wkbFlatten(ds->GetLayer(2)->GetGeomType()), wkbPoint);
  GDALClose(ds);
}

TEST(E2E, SdbdPublicLinkApiLayerAndFeatureCount) {
  testing::sdbd::SdbdHttpMock server;
  ASSERT_TRUE(server.StartAny()) << "failed to bind mock listener";
  ASSERT_NE(server.port(), 8020);
  const std::string url = std::string("sdbd://127.0.0.1:") +
                          std::to_string(server.port()) + "/?layer=t_rtree_demo";
  ASSERT_TRUE(content::LooksLikeSdbdLink(url));
  ASSERT_TRUE(content::TryOpenSpatialLink(url)) << content::SpatialLinkError();
  EXPECT_EQ(content::SpatialLinkKind(), "sdbd");
  EXPECT_GE(content::SpatialLinkLayerCount(), 1);
  ASSERT_NE(content::SdbdLinkLayer(), nullptr);
  EXPECT_GE(content::SdbdLinkLayer()->GetFeatureCount(), 1);
  ASSERT_NE(content::SdbdLinkFeatures(), nullptr);
  EXPECT_GE(content::SdbdLinkFeatures()->GetFeatureCount(), 1u);
  GDALDataset* ds = content::TakeSpatialLinkDataset();
  ASSERT_NE(ds, nullptr);
  EXPECT_GE(ds->GetLayerCount(), 1);
  content::CloseSdbdLink();
}

TEST(E2E, MapdFakeGoldenNoShapefileFallback) {
  GDALAllRegister();
  auto transport = sg::make_mapd_ready_transport<sg::mapd_e2e_fake_bind>();
  ASSERT_TRUE(content::LooksLikeMapdLink("mapd://127.0.0.1:8020"));
  ASSERT_TRUE(content::TryOpenMapdLink("mapd://127.0.0.1:8020", transport))
      << content::MapdLinkError();
  EXPECT_EQ(content::MapdLinkError(), "");
  EXPECT_GT(content::SharedMapdSession().LayerCount(), 0);
  GDALDataset* ds = content::SharedMapdSession().dataset();
  ASSERT_NE(ds, nullptr);
  ASSERT_NE(ds->GetLayer(0), nullptr);
  EXPECT_GT(ds->GetLayer(0)->GetFeatureCount(), 0);
  content::SharedMapdSession().Reset();
}

TEST(E2E, MapdLiveOrSkipNoShapefileFallback) {
  if (EnvTruthy("SG_MAPD_SKIP")) {
    GTEST_SKIP() << "SG_MAPD_SKIP";
  }
  const std::string url = "mapd://127.0.0.1:8020";
  ASSERT_TRUE(content::LooksLikeMapdLink(url));
  const bool ok = content::TryOpenSpatialLink(url);
  EXPECT_EQ(content::SpatialLinkKind(), "mapd");
  if (!ok) {
    EXPECT_FALSE(content::SpatialLinkError().empty());
    EXPECT_EQ(content::SpatialLinkLayerCount(), 0);
    EXPECT_EQ(content::TakeSpatialLinkDataset(), nullptr)
        << "mapd URL must not fall back to shapefile";
    if (EnvTruthy("SG_MAPD_REQUIRE")) {
      FAIL() << "mapd not_ready: " << content::SpatialLinkError();
    }
    GTEST_SKIP() << "mapd not listening: " << content::SpatialLinkError();
  }
  EXPECT_GT(content::SpatialLinkLayerCount(), 0);
  ASSERT_NE(content::TakeSpatialLinkDataset(), nullptr);
  content::SharedMapdSession().Reset();
}

#ifdef _WIN32
TEST(E2E, SmartgisStaysUpAndBindsThreeLayers) {
  if (EnvTruthy("SG_SMARTGIS_SKIP")) {
    GTEST_SKIP() << "SG_SMARTGIS_SKIP";
  }
  base::PathString exe_dir;
  ASSERT_TRUE(base::PathProvider(base::DIR_EXE, &exe_dir));
  const std::wstring exe = exe_dir + L"\\smartgis.exe";
  const std::wstring log_path = exe_dir + L"\\world_bind.log";
  if (!base::PathExists(exe)) {
    GTEST_SKIP() << "smartgis.exe not beside test";
  }
  DeleteFileW(log_path.c_str());

  STARTUPINFOW si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof(si);
  std::wstring cmd = L"\"" + exe + L"\"";
  std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
  cmd_buf.push_back(L'\0');
  ASSERT_TRUE(CreateProcessW(exe.c_str(), cmd_buf.data(), nullptr, nullptr,
                             FALSE, 0, nullptr, exe_dir.c_str(), &si, &pi))
      << "CreateProcess " << GetLastError();

  bool alive = true;
  std::string log;
  DWORD exit_code = STILL_ACTIVE;
  for (int i = 0; i < 40; ++i) {
    Sleep(200);
    if (GetExitCodeProcess(pi.hProcess, &exit_code) &&
        exit_code != STILL_ACTIVE) {
      alive = false;
      break;
    }
    std::ifstream in(base::UTF16ToUTF8(log_path));
    if (in) {
      log.assign((std::istreambuf_iterator<char>(in)),
                 std::istreambuf_iterator<char>());
      if (log.find("layers=3") != std::string::npos) {
        break;
      }
    }
  }

  if (GetExitCodeProcess(pi.hProcess, &exit_code) &&
      exit_code != STILL_ACTIVE) {
    alive = false;
  }
  if (alive) {
    Sleep(400);
    if (GetExitCodeProcess(pi.hProcess, &exit_code) &&
        exit_code != STILL_ACTIVE) {
      alive = false;
    }
  }
  if (log.empty()) {
    std::ifstream in(base::UTF16ToUTF8(log_path));
    if (in) {
      log.assign((std::istreambuf_iterator<char>(in)),
                 std::istreambuf_iterator<char>());
    }
  }

  if (alive) {
    TerminateProcess(pi.hProcess, 0);
    WaitForSingleObject(pi.hProcess, 5000);
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  EXPECT_TRUE(alive) << "smartgis.exe exited early code=" << exit_code;
  EXPECT_NE(log.find("layers=3"), std::string::npos) << log;
  EXPECT_NE(log.find("Polygon"), std::string::npos) << log;
  EXPECT_NE(log.find("Line String"), std::string::npos) << log;
  EXPECT_NE(log.find("Point"), std::string::npos) << log;
}
#endif
