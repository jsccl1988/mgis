// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "vista/pass/map/map_surface.h"

#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace vista {

namespace {

const int kKindMouse = 0;
const int kKindWheel = 1;
const int kActionMove = 1;
const int kActionPress = 2;
const int kActionRelease = 3;

const double kPi = 3.14159265358979323846;

const wchar_t kClassName[] = L"VistaMapSurface";

bool RegisterSurfaceClass() {
  static bool registered = false;
  if (registered)
    return true;
  WNDCLASSW wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = MapSurfaceProc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  registered = RegisterClassW(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
  return registered;
}


double MercX(double lon) {
  return (lon + 180.0) / 360.0;
}

double MercY(double lat) {
  double lat_rad = lat * kPi / 180.0;
  if (lat_rad > 1.48)
    lat_rad = 1.48;
  if (lat_rad < -1.48)
    lat_rad = -1.48;
  const double s = std::sin(lat_rad);
  const double c = std::cos(lat_rad);
  if (c == 0.0)
    return lat > 0.0 ? 0.0 : 1.0;
  return (1.0 - std::log((1.0 + s) / c) / kPi) / 2.0;
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

void Quad(double lon0, double lat0, double lon1, double lat1) {
  const double x0 = MercX(lon0);
  const double x1 = MercX(lon1);
  const double y0 = MercY(lat0);
  const double y1 = MercY(lat1);
  glVertex2d(x0, y0);
  glVertex2d(x1, y0);
  glVertex2d(x1, y1);
  glVertex2d(x0, y1);
}

// Mainland ring (lon, lat), closed. Matches offline MapLibre china source.
const double kChinaLon[] = {
    73.5, 76.0, 80.0, 85.0, 87.5, 90.0, 93.0, 97.0, 100.0, 104.0, 108.0,
    111.5, 115.0, 119.0, 122.5, 126.0, 130.0, 134.0, 133.0, 131.0, 128.5,
    126.0, 124.0, 122.5, 121.8, 121.5, 121.8, 122.0, 121.5, 121.0, 120.2,
    119.2, 118.0, 116.5, 114.5, 112.5, 110.5, 109.0, 107.5, 106.0, 104.5,
    102.5, 100.5, 98.8, 97.8, 97.2, 94.5, 91.5, 88.0, 85.0, 82.0, 79.5,
    77.5, 75.5, 73.5};
const double kChinaLat[] = {
    39.0, 40.5, 42.0, 45.0, 47.0, 48.5, 49.5, 50.5, 51.5, 53.0, 53.2,
    51.0, 48.5, 47.2, 47.5, 48.8, 48.5, 48.0, 46.0, 45.0, 43.0, 41.5,
    40.2, 39.5, 38.0, 36.5, 34.5, 32.0, 30.5, 29.0, 27.5, 26.0, 24.5,
    23.5, 22.5, 21.8, 21.2, 21.3, 21.8, 22.8, 22.6, 22.0, 21.5, 23.5,
    26.5, 28.2, 28.8, 28.2, 28.0, 28.5, 29.5, 31.0, 33.0, 35.5, 39.0};
const int kChinaN = static_cast<int>(sizeof(kChinaLon) / sizeof(kChinaLon[0]));

void EmitRing(const double* lon, const double* lat, int n, double shift) {
  for (int i = 0; i < n; ++i)
    glVertex2d(MercX(lon[i] + shift), MercY(lat[i]));
}

void DrawChinaOverview() {
  // Mainland + Taiwan / Hainan on ocean — same idea as MapLibre offline style.
  // Fill via centroid fans: GL_TRIANGLE_FAN from ring[0] spikes on this
  // concave coast (east-coast artifacts on mercator fallback).
  glColor3f(0.18f, 0.48f, 0.23f);
  for (int wrap = -1; wrap <= 1; ++wrap) {
    const double shift = static_cast<double>(wrap) * 360.0;
    double cx = 0.0;
    double cy = 0.0;
    for (int i = 0; i < kChinaN; ++i) {
      cx += MercX(kChinaLon[i] + shift);
      cy += MercY(kChinaLat[i]);
    }
    cx /= static_cast<double>(kChinaN);
    cy /= static_cast<double>(kChinaN);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < kChinaN - 1; ++i) {
      glVertex2d(cx, cy);
      glVertex2d(MercX(kChinaLon[i] + shift), MercY(kChinaLat[i]));
      glVertex2d(MercX(kChinaLon[i + 1] + shift), MercY(kChinaLat[i + 1]));
    }
    glEnd();
  }

  glColor3f(0.91f, 0.72f, 0.14f);
  glBegin(GL_QUADS);
  for (int wrap = -1; wrap <= 1; ++wrap) {
    const double shift = static_cast<double>(wrap) * 360.0;
    Quad(120.4 + shift, 21.8, 122.2 + shift, 25.4);  // Taiwan
    Quad(108.5 + shift, 18.1, 111.2 + shift, 20.2);  // Hainan
  }
  glEnd();

  glColor3f(0.08f, 0.24f, 0.11f);
  glLineWidth(2.0f);
  for (int wrap = -1; wrap <= 1; ++wrap) {
    const double shift = static_cast<double>(wrap) * 360.0;
    glBegin(GL_LINE_LOOP);
    EmitRing(kChinaLon, kChinaLat, kChinaN, shift);
    glEnd();
  }
}

}  // namespace

LRESULT CALLBACK MapSurfaceProc(HWND hwnd,
                                UINT msg,
                                WPARAM wparam,
                                LPARAM lparam) {
  if (msg == WM_NCCREATE) {
    CREATESTRUCTW* created = reinterpret_cast<CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(created->lpCreateParams));
  }
  MapSurface* surface =
      reinterpret_cast<MapSurface*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (msg) {
    case WM_ERASEBKGND:
      return 1;
    case WM_LBUTTONDOWN: {
      SetCapture(hwnd);
      SetFocus(hwnd);
      if (surface)
        surface->Notify(kKindMouse, kActionPress,
           static_cast<double>(static_cast<short>(LOWORD(lparam))),
           static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      return 0;
    }
    case WM_MOUSEMOVE:
      if (surface && GetCapture() == hwnd) {
        surface->Notify(kKindMouse, kActionMove,
             static_cast<double>(static_cast<short>(LOWORD(lparam))),
             static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      }
      return 0;
    case WM_LBUTTONUP:
      if (GetCapture() == hwnd)
        ReleaseCapture();
      if (surface)
        surface->Notify(kKindMouse, kActionRelease,
           static_cast<double>(static_cast<short>(LOWORD(lparam))),
           static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      return 0;
    case WM_MOUSEWHEEL: {
      POINT pt;
      pt.x = static_cast<short>(LOWORD(lparam));
      pt.y = static_cast<short>(HIWORD(lparam));
      ScreenToClient(hwnd, &pt);
      const double wheel =
          static_cast<double>(static_cast<short>(HIWORD(wparam))) / 120.0;
      if (surface)
        surface->Notify(kKindWheel, kActionMove, static_cast<double>(pt.x),
                        static_cast<double>(pt.y), 0.0, wheel);
      return 0;
    }
    default:
      break;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

MapSurface::MapSurface()
    : child_(NULL),
      dc_(NULL),
      gl_(NULL),
      in_present_(false),
      maplibre_disabled_(false),
      maplibre_(NULL),
      session_(NULL),
      open_(NULL),
      render_(NULL),
      close_(NULL),
      callback_(NULL),
      user_(NULL),
      token_(0),
      draw_calls_(0),
      width_(0),
      height_(0) {}

MapSurface::~MapSurface() {
  in_present_ = true;
  callback_ = NULL;
  if (session_ && close_)
    close_(session_);
  session_ = NULL;
  if (maplibre_)
    FreeLibrary(maplibre_);
  maplibre_ = NULL;
  if (gl_) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(gl_);
    gl_ = NULL;
  }
  if (dc_ && child_)
    ReleaseDC(child_, dc_);
  dc_ = NULL;
  if (child_)
    DestroyWindow(child_);
  child_ = NULL;
}

bool MapSurface::EnsureChild(HWND parent, int width, int height) {
  if (!parent || !IsWindow(parent))
    return false;
  if (!RegisterSurfaceClass())
    return false;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  if (!child_ || !IsWindow(child_)) {
    child_ = CreateWindowExW(0, kClassName, L"",
                             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                 WS_CLIPCHILDREN,
                             0, 0, width, height, parent, NULL,
                             GetModuleHandleW(NULL), this);
    if (!child_)
      return false;
  } else if (width != width_ || height != height_) {
    MoveWindow(child_, 0, 0, width, height, TRUE);
  }
  width_ = width;
  height_ = height;
  return true;
}

void MapSurface::Notify(int kind,
                        int action,
                        double x,
                        double y,
                        double wheel_dx,
                        double wheel_dy) {
  if (in_present_ || !callback_)
    return;
  callback_(user_, kind, action, x, y, wheel_dx, wheel_dy);
}

void MapSurface::Resize(int width, int height) {
  if (!child_)
    return;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  MoveWindow(child_, 0, 0, width, height, TRUE);
  width_ = width;
  height_ = height;
}

int MapSurface::Present(HWND parent,
                        int width,
                        int height,
                        double lon,
                        double lat,
                        double zoom,
                        double bearing,
                        double pitch,
                        MapPointerFn callback,
                        void* user) {
  if (in_present_)
    return 0;
  in_present_ = true;
  callback_ = callback;
  user_ = user;
  int code = 0;
  if (EnsureChild(parent, width, height)) {
    if (!maplibre_disabled_) {
      code = PresentMapLibre(width_, height_, lon, lat, zoom, bearing, pitch);
      if (code != 2)
        maplibre_disabled_ = true;
    }
    if (code != 2)
      code = PresentMercator(width_, height_, lon, lat, zoom);
  }
  in_present_ = false;
  return code;
}

int MapSurface::PresentMapLibre(int width,
                                int height,
                                double lon,
                                double lat,
                                double zoom,
                                double bearing,
                                double pitch) {
  if (!maplibre_) {
    wchar_t path[MAX_PATH];
    HMODULE self = NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&RegisterSurfaceClass),
                            &self))
      return 0;
    DWORD n = GetModuleFileNameW(self, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH)
      return 0;
    wchar_t* slash = path + n;
    while (slash > path && slash[-1] != L'\\' && slash[-1] != L'/')
      --slash;
    if (slash <= path)
      return 0;
    lstrcpynW(slash, L"mview_maplibre.dll",
              static_cast<int>(MAX_PATH - (slash - path)));
    maplibre_ = LoadLibraryW(path);
    if (!maplibre_)
      return 0;
    open_ = reinterpret_cast<int (*)(void**, int, int, float)>(
        GetProcAddress(maplibre_, "mln_session_open"));
    render_ = reinterpret_cast<int (*)(void*, HWND, int, int, double, double,
                                       double, double, double, unsigned*,
                                       int*)>(
        GetProcAddress(maplibre_, "mln_session_render"));
    close_ = reinterpret_cast<void (*)(void*)>(
        GetProcAddress(maplibre_, "mln_session_close"));
    if (!open_ || !render_ || !close_)
      return 0;
  }
  if (!session_) {
    void* created = NULL;
    if (!open_(&created, width, height, 1.0f) || !created)
      return 0;
    session_ = created;
  }
  unsigned token = 0;
  int draws = 0;
  if (!render_(session_, child_, width, height, lon, lat, zoom, bearing, pitch,
               &token, &draws))
    return 0;
  token_ = token;
  draw_calls_ = draws;
  return 2;
}

int MapSurface::PresentMercator(int width,
                                int height,
                                double lon,
                                double lat,
                                double zoom) {
  if (!dc_) {
    dc_ = GetDC(child_);
    if (!dc_)
      return 0;
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    const int format = ChoosePixelFormat(dc_, &pfd);
    if (format == 0 || !SetPixelFormat(dc_, format, &pfd))
      return 0;
    gl_ = wglCreateContext(dc_);
    if (!gl_)
      return 0;
  }
  if (!wglMakeCurrent(dc_, gl_))
    return 0;
  while (glGetError() != GL_NO_ERROR) {
  }

  glViewport(0, 0, width, height);
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.49f, 0.71f, 0.84f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const double scale = 256.0 * std::pow(2.0, zoom);
  const double cx = MercX(lon);
  const double cy = MercY(lat);
  const double half_w = (static_cast<double>(width) * 0.5) / scale;
  const double half_h = (static_cast<double>(height) * 0.5) / scale;
  glOrtho(cx - half_w, cx + half_w, cy + half_h, cy - half_h, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  DrawChinaOverview();

  std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 3);
  // Default pack alignment is 4. A width whose RGB row is not a multiple of 4
  // makes glReadPixels fail, and the frame is then treated as blank.
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
  bool varies = false;
  const unsigned hash =
      HashGrid(pixels.data(), width, height, width * 3, 3, &varies);
  const GLenum error = glGetError();
  const uint8_t* sample = pixels.data();
  const bool clear_only = std::abs(static_cast<int>(sample[0]) - 125) <= 3 &&
                          std::abs(static_cast<int>(sample[1]) - 181) <= 3 &&
                          std::abs(static_cast<int>(sample[2]) - 214) <= 3;
  const bool empty = (sample[0] | sample[1] | sample[2]) == 0;
  // Zoomed onto one land quad is a solid color, not a missed draw.
  if (error != GL_NO_ERROR || (!varies && (clear_only || empty))) {
    wglMakeCurrent(NULL, NULL);
    return 0;
  }
  if (!SwapBuffers(dc_)) {
    wglMakeCurrent(NULL, NULL);
    return 0;
  }
  token_ = hash;
  draw_calls_ = 2;
  wglMakeCurrent(NULL, NULL);
  return 1;
}

}  // namespace vista
