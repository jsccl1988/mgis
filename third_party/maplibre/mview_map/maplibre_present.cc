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

// Offline China-capable style (no network). World land blocks + mainland /
// Taiwan / Hainan outlines so ~105E/35N zoom 3.5–5 reads as a China map.
const char kStyleJson[] =
    "{"
    "\"version\":8,"
    "\"name\":\"mview-china\","
    "\"sources\":{"
    "\"land\":{"
    "\"type\":\"geojson\","
    "\"data\":{\"type\":\"FeatureCollection\",\"features\":["
    "{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[-130,25],[-130,50],[-60,50],[-60,25],[-130,25]]]}},"
    "{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[-10,35],[-10,60],[30,60],[30,35],[-10,35]]]}},"
    "{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[70,10],[70,55],[145,55],[145,10],[70,10]]]}}"
    "]}"
    "},"
    "\"china\":{"
    "\"type\":\"geojson\","
    "\"data\":{\"type\":\"FeatureCollection\",\"features\":["
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"mainland\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[73,39],[80,42],[87,47],[95,49],[105,49],[112,45],"
    "[119,43],[122,41],[124,40],[123,37],[122,32],[121,28],[120,25],"
    "[116,23],[110,21],[108,18],[104,22],[98,24],[91,28],[85,28],"
    "[80,30],[75,35],[73,39]]]}},"
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"taiwan\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[120.0,21.9],[122.0,21.9],[122.0,25.3],[120.0,25.3],[120.0,21.9]]]}},"
    "{\"type\":\"Feature\",\"properties\":{\"name\":\"hainan\"},\"geometry\":{\"type\":\"Polygon\","
    "\"coordinates\":[[[108.6,18.1],[111.1,18.1],[111.1,20.1],[108.6,20.1],[108.6,18.1]]]}}"
    "]}"
    "}"
    "},"
    "\"layers\":["
    "{\"id\":\"background\",\"type\":\"background\",\"paint\":{\"background-color\":\"#6fa8c9\"}},"
    "{\"id\":\"land\",\"type\":\"fill\",\"source\":\"land\",\"paint\":{\"fill-color\":\"#a8c47a\"}},"
    "{\"id\":\"china\",\"type\":\"fill\",\"source\":\"china\",\"paint\":{\"fill-color\":\"#2f7a3a\"}},"
    "{\"id\":\"china-outline\",\"type\":\"line\",\"source\":\"china\","
    "\"paint\":{\"line-color\":\"#1a4d24\",\"line-width\":1.2}}"
    "]"
    "}";

// Prefer demotiles when MVIEW_MAP_STYLE=demotiles (network). Default is the
// offline China GeoJSON above so the product shell always has a China map.
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

bool ApplyStyle(Session* session, const char* spec) {
  if (!session || !session->map)
    return false;
  if (!spec || !spec[0]) {
    session->map->getStyle().loadJSON(kStyleJson);
    return true;
  }
  if (_stricmp(spec, "demotiles") == 0) {
    session->map->getStyle().loadURL(kDemoTilesStyleUrl);
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
    // Default: offline China. Opt-in via MVIEW_MAP_STYLE=demotiles|url|path.
    char style_env[MAX_PATH] = {};
    const DWORD env_n =
        GetEnvironmentVariableA("MVIEW_MAP_STYLE", style_env, sizeof(style_env));
    const char* spec =
        (env_n > 0 && env_n < sizeof(style_env)) ? style_env : "";
    if (!ApplyStyle(held.get(), spec))
      ApplyStyle(held.get(), "");
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
