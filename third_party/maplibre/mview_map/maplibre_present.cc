// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// MapLibre Native still-image present. The headless WGL backend draws the
// frame; this file blits those pixels onto the caller's child HWND.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace {

// Offline China style (no network). Ocean background + mainland + Taiwan /
// Hainan only — no rectangular "world land" quads (those read as a wrong
// bounding box behind China at country zoom ~108E/34N, z 3.5–4.5).
const char kStyleJson[] =
    "{"
    "\"version\":8,"
    "\"name\":\"mview-china\","
    "\"sources\":{"
    "\"china\":{"
    "\"type\":\"geojson\","
    "\"data\":{\"type\":\"FeatureCollection\",\"features\":["
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"mainland\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[73.5,39.0],[76.0,40.5],[80.0,42.0],[85.0,45.0],"
    "[87.5,47.0],[90.0,48.5],[93.0,49.5],[97.0,50.5],[100.0,51.5],"
    "[104.0,53.0],[108.0,53.2],[111.5,51.0],[115.0,48.5],[119.0,47.2],"
    "[122.5,47.5],[126.0,48.8],[130.0,48.5],[134.0,48.0],[133.0,46.0],"
    "[131.0,45.0],[128.5,43.0],[126.0,41.5],[124.0,40.2],[122.5,39.5],"
    "[121.8,38.0],[121.5,36.5],[121.8,34.5],[122.0,32.0],[121.5,30.5],"
    "[121.0,29.0],[120.2,27.5],[119.2,26.0],[118.0,24.5],[116.5,23.5],"
    "[114.5,22.5],[112.5,21.8],[110.5,21.2],[109.0,21.3],[107.5,21.8],"
    "[106.0,22.8],[104.5,22.6],[102.5,22.0],[100.5,21.5],[98.8,23.5],"
    "[97.8,26.5],[97.2,28.2],[94.5,28.8],[91.5,28.2],[88.0,28.0],"
    "[85.0,28.5],[82.0,29.5],[79.5,31.0],[77.5,33.0],[75.5,35.5],"
    "[73.5,39.0]]]}}"
    "]}"
    "},"
    "\"islands\":{"
    "\"type\":\"geojson\","
    "\"data\":{\"type\":\"FeatureCollection\",\"features\":["
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"taiwan\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[120.5,21.7],[121.0,21.8],[121.6,22.3],[122.1,23.2],"
    "[122.2,24.2],[122.0,25.2],[121.5,25.5],[120.9,25.3],[120.4,24.5],"
    "[120.2,23.4],[120.3,22.4],[120.5,21.7]]]}},"
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"hainan\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[108.5,18.1],[109.5,18.0],[110.5,18.2],[111.2,18.8],"
    "[111.3,19.5],[110.8,20.2],[109.8,20.3],[108.9,20.0],[108.4,19.3],"
    "[108.5,18.1]]]}}"
    "]}"
    "}"
    "},"
    "\"layers\":["
    "{\"id\":\"background\",\"type\":\"background\",\"paint\":{\"background-color\":\"#6fa8c9\"}},"
    "{\"id\":\"china\",\"type\":\"fill\",\"source\":\"china\",\"paint\":{\"fill-color\":\"#2f7a3a\"}},"
    "{\"id\":\"china-outline\",\"type\":\"line\",\"source\":\"china\","
    "\"paint\":{\"line-color\":\"#143d1c\",\"line-width\":2.0}},"
    "{\"id\":\"islands\",\"type\":\"fill\",\"source\":\"islands\",\"paint\":{\"fill-color\":\"#e8b923\"}},"
    "{\"id\":\"islands-outline\",\"type\":\"line\",\"source\":\"islands\","
    "\"paint\":{\"line-color\":\"#6b4a00\",\"line-width\":2.2}}"
    "]"
    "}";
// Network OSM Bright (global OMT via OpenFreeMap). Cached under MapLibre's
// disk cache after first fetch. MVIEW_MAP_STYLE=offline keeps the GeoJSON
// mainland ring above. demotiles remains an opt-in world color demo.
const char kOpenFreeMapBrightUrl[] =
    "https://tiles.openfreemap.org/styles/bright";
const char kDemoTilesStyleUrl[] = "https://demotiles.maplibre.org/style.json";

std::string ReadFileUtf8(const char* path) {
  if (!path || !path[0])
    return std::string();
  FILE* file = nullptr;
  if (fopen_s(&file, path, "rb") != 0 || !file)
    return std::string();
  std::string out;
  char buf[4096];
  for (;;) {
    const size_t n = fread(buf, 1, sizeof(buf), file);
    if (n == 0)
      break;
    out.append(buf, n);
    if (out.size() > 2 * 1024 * 1024) {
      out.clear();
      break;
    }
  }
  fclose(file);
  return out;
}

class Observer : public mln::MapObserver {};

struct Session {
  Session() : loop(mln::util::RunLoop::Type::Default) {}

  mln::util::RunLoop loop;
  std::unique_ptr<mln::HeadlessFrontend> frontend;
  std::unique_ptr<Observer> observer;
  std::unique_ptr<mln::Map> map;
  int width;
  int height;

  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
};

bool ApplyStyle(Session* session, const char* spec) {
  if (!session || !session->map)
    return false;
  // Empty → OpenFreeMap Bright (real OSM). Explicit offline → GeoJSON ring.
  if (!spec || !spec[0]) {
    session->map->getStyle().loadURL(kOpenFreeMapBrightUrl);
    return true;
  }
  if (_stricmp(spec, "offline") == 0) {
    session->map->getStyle().loadJSON(kStyleJson);
    return true;
  }
  if (_stricmp(spec, "demotiles") == 0) {
    session->map->getStyle().loadURL(kDemoTilesStyleUrl);
    return true;
  }
  if (_stricmp(spec, "openfreemap") == 0 || _stricmp(spec, "bright") == 0) {
    session->map->getStyle().loadURL(kOpenFreeMapBrightUrl);
    return true;
  }
  if (_strnicmp(spec, "http://", 7) == 0 ||
      _strnicmp(spec, "https://", 8) == 0) {
    session->map->getStyle().loadURL(spec);
    return true;
  }
  const std::string json = ReadFileUtf8(spec);
  if (json.empty())
    return false;
  session->map->getStyle().loadJSON(json);
  return true;
}

unsigned HashGrid(const uint8_t* data,
                  int width,
                  int height,
                  int stride,
                  int channels,
                  bool* varies) {
  unsigned hash = 2166136261u;
  unsigned first = 0;
  bool have_first = false;
  bool different = false;
  const int step_x = width > 64 ? width / 64 : 1;
  const int step_y = height > 64 ? height / 64 : 1;
  for (int y = 0; y < height; y += step_y) {
    for (int x = 0; x < width; x += step_x) {
      const uint8_t* p = data + static_cast<size_t>(y) * stride +
                         static_cast<size_t>(x) * channels;
      const unsigned color = static_cast<unsigned>(p[0]) |
                             (static_cast<unsigned>(p[1]) << 8) |
                             (static_cast<unsigned>(p[2]) << 16);
      if (!have_first) {
        first = color;
        have_first = true;
      } else if (color != first) {
        different = true;
      }
      hash ^= p[0];
      hash *= 16777619u;
      hash ^= p[1];
      hash *= 16777619u;
      hash ^= p[2];
      hash *= 16777619u;
    }
  }
  if (varies)
    *varies = different;
  return hash == 0 ? 1u : hash;
}

bool Blit(HWND hwnd, const uint8_t* rgba, int width, int height) {
  if (!hwnd || !rgba || width <= 0 || height <= 0)
    return false;
  RECT client;
  if (!GetClientRect(hwnd, &client))
    return false;
  const int dest_w = client.right - client.left;
  const int dest_h = client.bottom - client.top;
  if (dest_w <= 0 || dest_h <= 0)
    return false;

  std::vector<uint8_t> bgra(static_cast<size_t>(width) * height * 4);
  for (int i = 0; i < width * height; ++i) {
    bgra[static_cast<size_t>(i) * 4 + 0] = rgba[static_cast<size_t>(i) * 4 + 2];
    bgra[static_cast<size_t>(i) * 4 + 1] = rgba[static_cast<size_t>(i) * 4 + 1];
    bgra[static_cast<size_t>(i) * 4 + 2] = rgba[static_cast<size_t>(i) * 4 + 0];
    bgra[static_cast<size_t>(i) * 4 + 3] = 255;
  }

  BITMAPINFO info;
  memset(&info, 0, sizeof(info));
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  HDC dc = GetDC(hwnd);
  if (!dc)
    return false;
  const int rows = StretchDIBits(dc, 0, 0, dest_w, dest_h, 0, 0, width, height,
                                 bgra.data(), &info, DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(hwnd, dc);
  return rows != 0 && rows != GDI_ERROR;
}

double ClampLat(double lat) {
  if (lat > 85.0)
    return 85.0;
  if (lat < -85.0)
    return -85.0;
  return lat;
}

bool NearRgb(const uint8_t* sample, int r, int g, int b, int tol) {
  if (!sample)
    return false;
  return std::abs(static_cast<int>(sample[0]) - r) <= tol &&
         std::abs(static_cast<int>(sample[1]) - g) <= tol &&
         std::abs(static_cast<int>(sample[2]) - b) <= tol;
}

// Reject only blank / failed frames. A solid China-green overview at country
// zoom is a valid offline style result (the polygon can fill the viewport).
bool IsBlankFrame(const uint8_t* sample, bool varies) {
  if (!sample)
    return true;
  const bool empty = (sample[0] | sample[1] | sample[2]) == 0;
  if (empty)
    return true;
  if (varies)
    return false;
  // White / near-white failed style, or MapLibre clear cyan with no layers.
  if (NearRgb(sample, 255, 255, 255, 8))
    return true;
  if (NearRgb(sample, 125, 181, 214, 8))
    return true;
  return false;
}

}  // namespace

extern "C" __declspec(dllexport) int mln_session_open(void** session,
                                                      int width,
                                                      int height,
                                                      float pixel_ratio) {
  if (!session || width < 2 || height < 2)
    return 0;
  *session = NULL;
  if (!(pixel_ratio > 0.0f))
    pixel_ratio = 1.0f;
  try {
    std::unique_ptr<Session> held(new Session());
    held->width = width;
    held->height = height;
    held->frontend.reset(new mln::HeadlessFrontend(
        mln::Size(static_cast<uint32_t>(width), static_cast<uint32_t>(height)),
        pixel_ratio));
    held->observer.reset(new Observer());

    char temp[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, temp);
    std::string cache = n > 0 ? std::string(temp, n) : std::string(".\\");
    cache += "mview-maplibre-cache";
    CreateDirectoryA(cache.c_str(), NULL);

    mln::MapOptions options;
    options.withMapMode(mln::MapMode::Static)
        .withSize(mln::Size(static_cast<uint32_t>(width),
                            static_cast<uint32_t>(height)))
        .withPixelRatio(pixel_ratio);
    mln::ResourceOptions resources;
    resources.withCachePath(cache).withApiKey("mview");
    mln::ClientOptions client;
    client.withName("mview").withVersion("0");

    held->map.reset(new mln::Map(*held->frontend, *held->observer, options,
                                 resources, client));
    // Default: OpenFreeMap Bright. Opt-in offline / demotiles / url / path.
    char style_env[MAX_PATH] = {};
    const DWORD env_n =
        GetEnvironmentVariableA("MVIEW_MAP_STYLE", style_env, sizeof(style_env));
    const char* spec =
        (env_n > 0 && env_n < sizeof(style_env)) ? style_env : "";
    if (!ApplyStyle(held.get(), spec)) {
      // Network style failed — keep a China-shaped offline map.
      ApplyStyle(held.get(), "offline");
    }
    *session = held.release();
    return 1;
  } catch (const std::exception&) {
    return 0;
  } catch (...) {
    return 0;
  }
}

extern "C" __declspec(dllexport) int mln_session_set_style(void* raw,
                                                           const char* spec) {
  Session* session = static_cast<Session*>(raw);
  if (!session || !session->map)
    return 0;
  try {
    return ApplyStyle(session, spec) ? 1 : 0;
  } catch (const std::exception&) {
    return 0;
  } catch (...) {
    return 0;
  }
}

extern "C" __declspec(dllexport) void mln_session_close(void* session) {
  delete static_cast<Session*>(session);
}

extern "C" __declspec(dllexport) int mln_session_render(void* raw,
                                                        HWND hwnd,
                                                        int width,
                                                        int height,
                                                        double lon,
                                                        double lat,
                                                        double zoom,
                                                        double bearing,
                                                        double pitch,
                                                        unsigned* token,
                                                        int* draw_calls) {
  if (token)
    *token = 0;
  if (draw_calls)
    *draw_calls = 0;
  Session* session = static_cast<Session*>(raw);
  if (!session || !session->map || !session->frontend || !hwnd)
    return 0;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  try {
    if (width != session->width || height != session->height) {
      const mln::Size size(static_cast<uint32_t>(width),
                           static_cast<uint32_t>(height));
      session->frontend->setSize(size);
      session->map->setSize(size);
      session->width = width;
      session->height = height;
    }
    mln::CameraOptions camera;
    camera.center = mln::LatLng(ClampLat(lat), lon);
    camera.zoom = zoom;
    camera.bearing = bearing;
    camera.pitch = pitch;
    session->map->jumpTo(camera);

    mln::HeadlessFrontend::RenderResult result =
        session->frontend->render(*session->map);
    if (!result.image.valid() || result.stats.numDrawCalls <= 0)
      return 0;
    bool varies = false;
    const unsigned hash =
        HashGrid(result.image.data.get(), result.image.size.width,
                 result.image.size.height,
                 static_cast<int>(result.image.stride()),
                 static_cast<int>(result.image.channels), &varies);
    const uint8_t* sample = result.image.data.get();
    if (IsBlankFrame(sample, varies))
      return 0;
    if (!Blit(hwnd, result.image.data.get(), result.image.size.width,
              result.image.size.height))
      return 0;
    if (token)
      *token = hash;
    if (draw_calls)
      *draw_calls = result.stats.numDrawCalls;
    return 1;
  } catch (const std::exception&) {
    return 0;
  } catch (...) {
    return 0;
  }
}
