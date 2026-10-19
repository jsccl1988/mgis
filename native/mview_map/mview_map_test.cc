// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "native/mview_map/camera_follow.h"
#include "native/mview_map/mview_map.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "tool/workspace.h"

namespace {

std::string WideToUtf8(const std::wstring& wide) {
  if (wide.empty())
    return std::string();
  const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                        static_cast<int>(wide.size()), NULL, 0,
                                        NULL, NULL);
  if (bytes <= 0)
    return std::string();
  std::string utf8(static_cast<size_t>(bytes), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                      &utf8[0], bytes, NULL, NULL);
  return utf8;
}

std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty())
    return std::wstring();
  const int chars = MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                                        static_cast<int>(utf8.size()), NULL, 0);
  if (chars <= 0)
    return std::wstring();
  std::wstring wide(static_cast<size_t>(chars), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
                      &wide[0], chars);
  return wide;
}

bool FileExists(const std::wstring& path) {
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::wstring RepoRoot() {
  wchar_t module[MAX_PATH];
  const DWORD n = GetModuleFileNameW(NULL, module, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return std::wstring();
  std::wstring dir(module, n);
  for (int i = 0; i < 8; ++i) {
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
      break;
    dir.erase(slash);
    if (FileExists(dir + L"\\BUILD.gn") &&
        FileExists(dir + L"\\native\\mview_map\\camera_follow.h"))
      return dir;
  }
  return std::wstring();
}

void EnsureDir(const std::wstring& path) {
  if (path.empty())
    return;
  size_t pos = 0;
  if (path.size() >= 2 && path[1] == L':')
    pos = 2;
  while (pos < path.size()) {
    const size_t next = path.find_first_of(L"\\/", pos + 1);
    const std::wstring prefix = path.substr(0, next);
    if (!prefix.empty() && prefix[prefix.size() - 1] != L':')
      CreateDirectoryW(prefix.c_str(), NULL);
    if (next == std::wstring::npos)
      break;
    pos = next;
  }
}

bool WriteText(const std::wstring& path, const std::string& text) {
  std::ofstream out(WideToUtf8(path).c_str(), std::ios::out | std::ios::binary);
  if (!out)
    return false;
  out << text;
  return out.good();
}

void EnableWindowDpiAwareness() {
  HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (!user32)
    user32 = LoadLibraryW(L"user32.dll");
  if (!user32)
    return;
  typedef BOOL(WINAPI * SetContextFn)(void*);
  SetContextFn set_context = reinterpret_cast<SetContextFn>(
      GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
  // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 is (HANDLE)-4.
  if (set_context &&
      set_context(reinterpret_cast<void*>(static_cast<INT_PTR>(-4))))
    return;
  typedef BOOL(WINAPI * SetAwareFn)();
  SetAwareFn set_aware =
      reinterpret_cast<SetAwareFn>(GetProcAddress(user32, "SetProcessDPIAware"));
  if (set_aware)
    set_aware();
}

double WindowPixelRatio(HWND hwnd) {
  typedef UINT(WINAPI * GetDpiForWindowFn)(HWND);
  GetDpiForWindowFn get_dpi = NULL;
  HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32)
    get_dpi = reinterpret_cast<GetDpiForWindowFn>(
        GetProcAddress(user32, "GetDpiForWindow"));
  UINT dpi = 96;
  if (get_dpi && hwnd)
    dpi = get_dpi(hwnd);
  if (dpi == 0)
    dpi = 96;
  return static_cast<double>(dpi) / 96.0;
}

tool::InputEvent MouseEvent(tool::MouseAction action, double x, double y) {
  tool::InputEvent event;
  event.kind = tool::kMouse;
  event.action = action;
  event.x_px = x;
  event.y_px = y;
  event.view_id = "map";
  return event;
}

double DegreesPerPixel(double zoom) {
  return 360.0 / (256.0 * std::pow(2.0, zoom));
}

void ReadCamera(MvmMap* map, mview_map::MapCameraState* state) {
  mvm_get_camera(map, &state->lon, &state->lat, &state->zoom, &state->bearing,
                 &state->pitch);
}

// Only camera writer the gesture path is allowed to call.
void SetCamera(MvmMap* map, const mview_map::MapCameraState& state,
               const char* path, std::string* log, int* calls) {
  *log += path;
  *log += " mvm_set_camera\n";
  ++(*calls);
  mvm_set_camera(map, state.lon, state.lat, state.zoom, state.bearing,
                 state.pitch);
}

}  // namespace

TEST(MviewMap, Follow2dAndToolPath) {
  const std::wstring root = RepoRoot();
  ASSERT_FALSE(root.empty());

  EnableWindowDpiAwareness();
  HWND hwnd = CreateWindowExW(0, L"STATIC", L"mview-map-dpi", WS_OVERLAPPED, 0,
                              0, 0, 0, NULL, NULL, GetModuleHandleW(NULL),
                              NULL);
  ASSERT_TRUE(hwnd != NULL);
  const double pixel_ratio = WindowPixelRatio(hwnd);

  MvmMap* map = mvm_create(hwnd, 800, 600, pixel_ratio);
  ASSERT_TRUE(map != NULL);

  double lon_error = 0.0;
  double lat_error = 0.0;
  double height_error = 0.0;

  const double samples[][5] = {
      {0.0, 0.0, 0.0, 0.0, 0.0},
      {116.391234567, 39.907654321, 10.5, 15.0, 20.0},
      {-122.4194, 37.7749, 4.25, -30.0, 45.0},
      {179.123456, -33.8688, 15.5, 90.0, 5.0},
      {0.0, 0.0, 22.0, 0.0, 0.0},
      {12.5, -45.25, 12.0, 180.0, 10.0},
  };
  const int sample_count = sizeof(samples) / sizeof(samples[0]);
  for (int i = 0; i < sample_count; ++i) {
    mview_map::MapCameraState map_cam;
    map_cam.lon = samples[i][0];
    map_cam.lat = samples[i][1];
    map_cam.zoom = samples[i][2];
    map_cam.bearing = samples[i][3];
    map_cam.pitch = samples[i][4];

    mview_map::ViewCameraState view;
    mview_map::Follow2dMapToView(map_cam, &view);
    mview_map::MapCameraState round;
    mview_map::Follow2dViewToMap(view, &round);
    mview_map::ViewCameraState view_again;
    mview_map::Follow2dMapToView(round, &view_again);

    lon_error = std::max(lon_error, std::fabs(round.lon - map_cam.lon));
    lat_error = std::max(lat_error, std::fabs(round.lat - map_cam.lat));
    height_error =
        std::max(height_error, std::fabs(view_again.height_m - view.height_m));
    lon_error = std::max(lon_error, std::fabs(view.lon - map_cam.lon));
    lat_error = std::max(lat_error, std::fabs(view.lat - map_cam.lat));
    EXPECT_DOUBLE_EQ(view.heading, map_cam.bearing);
    EXPECT_DOUBLE_EQ(view.pitch, map_cam.pitch);
  }

  const double mid_heights[] = {1000.0, 25000.0, 40075016.685578488};
  for (int i = 0; i < 3; ++i) {
    mview_map::ViewCameraState view;
    view.lon = 8.5;
    view.lat = 47.3;
    view.height_m = mid_heights[i];
    view.heading = 12.0;
    view.pitch = 3.0;
    mview_map::MapCameraState map_cam;
    mview_map::Follow2dViewToMap(view, &map_cam);
    mview_map::ViewCameraState back;
    mview_map::Follow2dMapToView(map_cam, &back);
    lon_error = std::max(lon_error, std::fabs(back.lon - view.lon));
    lat_error = std::max(lat_error, std::fabs(back.lat - view.lat));
    height_error =
        std::max(height_error, std::fabs(back.height_m - view.height_m));
  }

  for (int i = 0; i < mview_map::ZoomTableSize() - 1; ++i) {
    const double h0 = mview_map::ZoomTableHeight(i);
    const double h1 = mview_map::ZoomTableHeight(i + 1);
    EXPECT_GT(h0, h1);
  }

  std::string log;
  int set_calls = 0;
  mview_map::MapCameraState camera;
  camera.lon = 116.3;
  camera.lat = 39.9;
  camera.zoom = 12.0;
  camera.bearing = 5.0;
  camera.pitch = 0.0;
  log += "activate_tool view.pan\n";
  SetCamera(map, camera, "init", &log, &set_calls);

  tool::Workspace workspace("map");
  ASSERT_TRUE(workspace.execute("view.pan"));
  ASSERT_TRUE(workspace.stack().current() != NULL);
  EXPECT_STREQ("view.pan", workspace.stack().current()->id());

  const tool::InputEvent press = MouseEvent(tool::kMousePress, 100.0, 200.0);
  const tool::InputEvent move = MouseEvent(tool::kMouseMove, 140.0, 180.0);
  const tool::InputEvent release = MouseEvent(tool::kMouseRelease, 140.0, 180.0);
  EXPECT_TRUE(workspace.dispatch(press));
  log += "view.pan anchor\n";
  EXPECT_TRUE(workspace.dispatch(move));

  const double deg = DegreesPerPixel(camera.zoom);
  const double dx = move.x_px - press.x_px;
  const double dy = move.y_px - press.y_px;
  mview_map::MapCameraState panned = camera;
  panned.lon = camera.lon - dx * deg;
  panned.lat = camera.lat + dy * deg;
  SetCamera(map, panned, "view.pan", &log, &set_calls);
  EXPECT_TRUE(workspace.dispatch(release));

  tool::InputEvent wheel;
  wheel.kind = tool::kWheel;
  wheel.wheel_dy = 1.0;
  wheel.view_id = "map";
  EXPECT_TRUE(workspace.always_on("wheel.zoom")->on_input(wheel));
  EXPECT_FALSE(workspace.stack().current()->on_input(wheel));
  EXPECT_TRUE(workspace.dispatch(wheel));
  mview_map::MapCameraState zoomed = panned;
  zoomed.zoom = panned.zoom + wheel.wheel_dy;
  SetCamera(map, zoomed, "wheel.zoom", &log, &set_calls);

  mview_map::MapCameraState got;
  ReadCamera(map, &got);
  lon_error = std::max(lon_error, std::fabs(got.lon - zoomed.lon));
  lat_error = std::max(lat_error, std::fabs(got.lat - zoomed.lat));
  mview_map::ViewCameraState view;
  mview_map::Follow2dMapToView(got, &view);
  mview_map::MapCameraState from_view;
  mview_map::Follow2dViewToMap(view, &from_view);
  mview_map::ViewCameraState view_back;
  mview_map::Follow2dMapToView(from_view, &view_back);
  height_error =
      std::max(height_error, std::fabs(view_back.height_m - view.height_m));
  lon_error = std::max(lon_error, std::fabs(from_view.lon - got.lon));
  lat_error = std::max(lat_error, std::fabs(from_view.lat - got.lat));

  const std::string style =
      WideToUtf8(root + L"\\third_party\\maplibre\\example\\style_background.json");
  mvm_set_style_url(map, style.c_str());
  char layers[4096];
  const int listed = mvm_list_layers(map, layers, static_cast<int>(sizeof(layers)));
  EXPECT_GT(listed, 0);
  EXPECT_NE(std::string(layers).find("\"id\":\"background\""), std::string::npos);

  const std::wstring geojson = root + L"\\third_party\\maplibre\\example\\style_background.json";
  mvm_add_geojson(map, "roads", geojson.c_str());
  mvm_set_layer_visible(map, "roads", 0);
  log += "layer.visible id=roads\n";
  const int listed_after =
      mvm_list_layers(map, layers, static_cast<int>(sizeof(layers)));
  EXPECT_GT(listed_after, 0);
  const std::string layer_json(layers);
  EXPECT_NE(layer_json.find("\"id\":\"roads\""), std::string::npos);
  EXPECT_NE(layer_json.find("\"visible\":0"), std::string::npos);
  EXPECT_EQ(layer_json.find("coordinates"), std::string::npos);

  log += "side_path none\n";
  EXPECT_EQ(log.find("SIDE_PATH"), std::string::npos);
  EXPECT_EQ(log.find("wheel.zoom mvm_set_camera"), log.rfind("wheel.zoom mvm_set_camera"));
  EXPECT_EQ(std::string::npos, log.find("wheel.zoom view.pan"));
  EXPECT_EQ(std::string::npos, log.find("view.pan wheel.zoom"));
  EXPECT_NE(log.find("view.pan mvm_set_camera"), std::string::npos);
  EXPECT_NE(log.find("wheel.zoom mvm_set_camera"), std::string::npos);
  EXPECT_EQ(3, set_calls);

  const std::wstring harness = root + L"\\out\\harness\\browser.map";
  EnsureDir(harness);
  ASSERT_TRUE(WriteText(harness + L"\\map.log", log));

  EXPECT_LE(lon_error, 1e-6);
  EXPECT_LE(lat_error, 1e-6);
  EXPECT_LE(height_error, 1.0);

  if (!HasFailure()) {
    char mark[1024];
    std::snprintf(mark, sizeof(mark),
                  "{\n"
                  "  \"suite\": \"browser.map\",\n"
                  "  \"expect_camera\": \"pass\",\n"
                  "  \"pixel_ratio\": %.8f,\n"
                  "  \"sources\": \"abi-stub\",\n"
                  "  \"note\": \"sources: abi-stub\",\n"
                  "  \"activate_tool\": \"view.pan\",\n"
                  "  \"lon_error_deg\": %.17g,\n"
                  "  \"lat_error_deg\": %.17g,\n"
                  "  \"height_error_m\": %.17g\n"
                  "}\n",
                  pixel_ratio, lon_error, lat_error, height_error);
    EXPECT_TRUE(WriteText(harness + L"\\mark.json", mark));
  }

  mvm_destroy(map);
  DestroyWindow(hwnd);
}
