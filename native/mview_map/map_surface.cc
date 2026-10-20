// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "native/mview_map/map_surface.h"

#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace {

const int kKindMouse = 0;
const int kKindWheel = 1;
const int kActionMove = 1;
const int kActionPress = 2;
const int kActionRelease = 3;

const double kPi = 3.14159265358979323846;

const wchar_t kClassName[] = L"MViewMapSurface";

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
                        MvmPointerFn callback,
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

  glColor3f(0.56f, 0.74f, 0.40f);
  glBegin(GL_QUADS);
  for (int wrap = -1; wrap <= 1; ++wrap) {
    const double shift = static_cast<double>(wrap) * 360.0;
    Quad(-130.0 + shift, 25.0, -60.0 + shift, 50.0);
    Quad(-10.0 + shift, 35.0, 30.0 + shift, 60.0);
    Quad(70.0 + shift, 10.0, 140.0 + shift, 50.0);
    Quad(-20.0 + shift, -35.0, 50.0 + shift, 35.0);
    Quad(110.0 + shift, -45.0, 155.0 + shift, -10.0);
  }
  glEnd();

  glColor3f(0.20f, 0.38f, 0.52f);
  glBegin(GL_LINES);
  for (int lon_line = -180; lon_line <= 180; lon_line += 30) {
    const double x = MercX(static_cast<double>(lon_line));
    glVertex2d(x, 0.0);
    glVertex2d(x, 1.0);
  }
  for (int lat_line = -60; lat_line <= 60; lat_line += 30) {
    const double y = MercY(static_cast<double>(lat_line));
    glVertex2d(-1.0, y);
    glVertex2d(2.0, y);
  }
  glEnd();

  std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 3);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
  bool varies = false;
  const unsigned hash =
      HashGrid(pixels.data(), width, height, width * 3, 3, &varies);
  const GLenum error = glGetError();
  if (!varies || error != GL_NO_ERROR) {
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
