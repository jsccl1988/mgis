// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/present/flycube_host.h"

#include <GL/gl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace content {

LRESULT CALLBACK FlyCubeHostProc(HWND hwnd,
                                 UINT msg,
                                 WPARAM wparam,
                                 LPARAM lparam);

namespace {

const int kStacks = 36;
const int kSlices = 72;
const wchar_t kClassName[] = L"MViewFlyCubeHost";

struct Vert {
  float x, y, z;
  float u, v;
  unsigned char r, g, b, a;
};

std::vector<Vert> g_verts;
std::vector<unsigned short> g_indices;
bool g_mesh = false;

const float kChinaLon[] = {
    73.5f, 76.0f, 80.0f, 85.0f, 87.5f, 90.0f, 93.0f, 97.0f, 100.0f, 104.0f,
    108.0f, 111.5f, 115.0f, 119.0f, 122.5f, 126.0f, 130.0f, 134.0f, 133.0f,
    131.0f, 128.5f, 126.0f, 124.0f, 122.5f, 121.8f, 121.5f, 121.8f, 122.0f,
    121.5f, 121.0f, 120.2f, 119.2f, 118.0f, 116.5f, 114.5f, 112.5f, 110.5f,
    109.0f, 107.5f, 106.0f, 104.5f, 102.5f, 100.5f, 98.8f, 97.8f, 97.2f,
    94.5f, 91.5f, 88.0f, 85.0f, 82.0f, 79.5f, 77.5f, 75.5f, 73.5f};
const float kChinaLat[] = {
    39.0f, 40.5f, 42.0f, 45.0f, 47.0f, 48.5f, 49.5f, 50.5f, 51.5f, 53.0f,
    53.2f, 51.0f, 48.5f, 47.2f, 47.5f, 48.8f, 48.5f, 48.0f, 46.0f, 45.0f,
    43.0f, 41.5f, 40.2f, 39.5f, 38.0f, 36.5f, 34.5f, 32.0f, 30.5f, 29.0f,
    27.5f, 26.0f, 24.5f, 23.5f, 22.5f, 21.8f, 21.2f, 21.3f, 21.8f, 22.8f,
    22.6f, 22.0f, 21.5f, 23.5f, 26.5f, 28.2f, 28.8f, 28.2f, 28.0f, 28.5f,
    29.5f, 31.0f, 33.0f, 35.5f, 39.0f};
const int kChinaN = static_cast<int>(sizeof(kChinaLon) / sizeof(kChinaLon[0]));

bool PointInChina(float lon_deg, float lat_deg) {
  bool inside = false;
  for (int i = 0, j = kChinaN - 1; i < kChinaN; j = i++) {
    const float yi = kChinaLat[i];
    const float yj = kChinaLat[j];
    const float xi = kChinaLon[i];
    const float xj = kChinaLon[j];
    const bool hit = ((yi > lat_deg) != (yj > lat_deg)) &&
                     (lon_deg < (xj - xi) * (lat_deg - yi) / (yj - yi + 1e-6f) + xi);
    if (hit)
      inside = !inside;
  }
  if (lon_deg > 120.3f && lon_deg < 122.3f && lat_deg > 21.6f && lat_deg < 25.5f)
    return true;
  if (lon_deg > 108.4f && lon_deg < 111.3f && lat_deg > 18.0f && lat_deg < 20.3f)
    return true;
  return inside;
}

float ChinaDemShade(float lon_deg, float lat_deg) {
  const float west = (std::max)(0.0f, (std::min)(1.0f, (100.0f - lon_deg) / 25.0f));
  const float north = (std::max)(0.0f, (std::min)(1.0f, (lat_deg - 25.0f) / 25.0f));
  return 0.25f + 0.55f * west + 0.20f * north;
}

void EnsureMesh() {
  if (g_mesh)
    return;
  g_verts.resize(static_cast<size_t>(kStacks + 1) * (kSlices + 1));
  for (int y = 0; y <= kStacks; ++y) {
    const float v = static_cast<float>(y) / static_cast<float>(kStacks);
    const float lat = (0.5f - v) * 3.14159265f;
    const float cos_lat = std::cos(lat);
    const float sin_lat = std::sin(lat);
    const float lat_deg = lat * 180.0f / 3.14159265f;
    for (int x = 0; x <= kSlices; ++x) {
      const float u = static_cast<float>(x) / static_cast<float>(kSlices);
      const float lon = u * 2.0f * 3.14159265f - 3.14159265f;
      const float lon_deg = lon * 180.0f / 3.14159265f;
      Vert& vert = g_verts[static_cast<size_t>(y) * (kSlices + 1) + x];
      vert.x = cos_lat * std::cos(lon);
      vert.y = sin_lat;
      vert.z = cos_lat * std::sin(lon);
      vert.u = u;
      vert.v = v;
      if (PointInChina(lon_deg, lat_deg)) {
        const float h = ChinaDemShade(lon_deg, lat_deg);
        vert.r = static_cast<unsigned char>(50 + 90 * h);
        vert.g = static_cast<unsigned char>(140 - 60 * h);
        vert.b = static_cast<unsigned char>(55 + 15 * h);
      } else {
        vert.r = 40;
        vert.g = 90;
        vert.b = 150;
      }
      vert.a = 255;
    }
  }
  g_indices.reserve(static_cast<size_t>(kStacks) * kSlices * 6);
  for (int y = 0; y < kStacks; ++y) {
    for (int x = 0; x < kSlices; ++x) {
      const unsigned short i0 =
          static_cast<unsigned short>(y * (kSlices + 1) + x);
      const unsigned short i1 =
          static_cast<unsigned short>(i0 + (kSlices + 1));
      g_indices.push_back(i0);
      g_indices.push_back(i1);
      g_indices.push_back(static_cast<unsigned short>(i1 + 1));
      g_indices.push_back(i0);
      g_indices.push_back(static_cast<unsigned short>(i1 + 1));
      g_indices.push_back(static_cast<unsigned short>(i0 + 1));
    }
  }
  g_mesh = true;
}

void Perspective(float fovy_deg, float aspect, float znear, float zfar) {
  const float f = 1.0f / std::tan(fovy_deg * 3.14159265f / 360.0f);
  float m[16];
  memset(m, 0, sizeof(m));
  m[0] = f / aspect;
  m[5] = f;
  m[10] = (zfar + znear) / (znear - zfar);
  m[11] = -1.0f;
  m[14] = (2.0f * zfar * znear) / (znear - zfar);
  glLoadMatrixf(m);
}

unsigned HashGrid(const uint8_t* data, int width, int height, bool* varies) {
  unsigned hash = 2166136261u;
  unsigned first = 0;
  bool have_first = false;
  bool different = false;
  const int step_x = width > 64 ? width / 64 : 1;
  const int step_y = height > 64 ? height / 64 : 1;
  for (int y = 0; y < height; y += step_y) {
    for (int x = 0; x < width; x += step_x) {
      const uint8_t* p = data + (static_cast<size_t>(y) * width + x) * 3;
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

bool RegisterHostClass() {
  static bool registered = false;
  if (registered)
    return true;
  WNDCLASSW wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = FlyCubeHostProc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  registered = RegisterClassW(&wc) != 0 ||
               GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
  return registered;
}

void SyncOrbitFromCamera(double lon, double lat, double height_m,
                         double* yaw, double* pitch, double* distance) {
  *yaw = lon * 3.14159265 / 180.0;
  *pitch = lat * 3.14159265 / 180.0 * 0.5;
  if (*pitch > 1.2)
    *pitch = 1.2;
  if (*pitch < -1.2)
    *pitch = -1.2;
  double d = 1.4 + std::log10(std::max(height_m, 100.0)) * 0.55;
  if (d < 1.4)
    d = 1.4;
  if (d > 8.0)
    d = 8.0;
  *distance = d;
}

}  // namespace

LRESULT CALLBACK FlyCubeHostProc(HWND hwnd,
                                 UINT msg,
                                 WPARAM wparam,
                                 LPARAM lparam) {
  if (msg == WM_NCCREATE) {
    CREATESTRUCTW* created = reinterpret_cast<CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(created->lpCreateParams));
  }
  FlyCubeHost* host =
      reinterpret_cast<FlyCubeHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const int kKindMouse = 0;
  const int kKindWheel = 1;
  const int kActionMove = 1;
  const int kActionPress = 2;
  const int kActionRelease = 3;
  switch (msg) {
    case WM_ERASEBKGND:
      return 1;
    case WM_LBUTTONDOWN:
      SetCapture(hwnd);
      SetFocus(hwnd);
      if (host) {
        host->right_down_ = false;
        host->NotifyPointer(
            kKindMouse, kActionPress,
            static_cast<double>(static_cast<short>(LOWORD(lparam))),
            static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      }
      return 0;
    case WM_RBUTTONDOWN:
      SetCapture(hwnd);
      SetFocus(hwnd);
      if (host) {
        host->right_down_ = true;
        host->NotifyPointer(
            kKindMouse, kActionPress,
            static_cast<double>(static_cast<short>(LOWORD(lparam))),
            static_cast<double>(static_cast<short>(HIWORD(lparam))), 1.0, 0.0);
      }
      return 0;
    case WM_MOUSEMOVE:
      if (host && GetCapture() == hwnd) {
        host->NotifyPointer(
            kKindMouse, kActionMove,
            static_cast<double>(static_cast<short>(LOWORD(lparam))),
            static_cast<double>(static_cast<short>(HIWORD(lparam))),
            host->right_down_ ? 1.0 : 0.0, 0.0);
      }
      return 0;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      if (GetCapture() == hwnd)
        ReleaseCapture();
      if (host) {
        const bool right = host->right_down_ || msg == WM_RBUTTONUP;
        host->right_down_ = false;
        host->NotifyPointer(
            kKindMouse, kActionRelease,
            static_cast<double>(static_cast<short>(LOWORD(lparam))),
            static_cast<double>(static_cast<short>(HIWORD(lparam))),
            right ? 1.0 : 0.0, 0.0);
      }
      return 0;
    case WM_MOUSEWHEEL: {
      POINT pt;
      pt.x = static_cast<short>(LOWORD(lparam));
      pt.y = static_cast<short>(HIWORD(lparam));
      ScreenToClient(hwnd, &pt);
      const double wheel =
          static_cast<double>(static_cast<short>(HIWORD(wparam))) / 120.0;
      if (host)
        host->NotifyPointer(kKindWheel, kActionMove, static_cast<double>(pt.x),
                            static_cast<double>(pt.y), 0.0, wheel);
      return 0;
    }
    default:
      break;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

FlyCubeHost::FlyCubeHost()
    : child_(NULL),
      dc_(NULL),
      gl_(NULL),
      in_present_(false),
      mesh_ready_(false),
      dx12_(false),
      world_(NULL),
      lon_(105.0),
      lat_(35.0),
      height_m_(1.2e7),
      yaw_(0.4),
      pitch_(0.3),
      distance_(3.2),
      width_(0),
      height_(0),
      token_(0),
      backend_("flycube-gl"),
      pointer_fn_(NULL),
      pointer_user_(NULL),
      right_down_(false) {
  SyncOrbitFromCamera(lon_, lat_, height_m_, &yaw_, &pitch_, &distance_);
}

FlyCubeHost::~FlyCubeHost() {
  Destroy();
}

bool FlyCubeHost::TryWorldCreate(HWND parent, int width, int height) {
  world_ = new vista::WorldPass();
  if (!world_->Create(parent, width, height)) {
    delete world_;
    world_ = NULL;
    return false;
  }
  child_ = world_->hwnd();
  if (!child_) {
    delete world_;
    world_ = NULL;
    return false;
  }
  // WorldPass::Create already frames a China DEM overview (draped ENU orbit or
  // sphere fallback). Do not stomp with the host's full-Earth height (~1.2e7 m)
  // — that pulls the camera so far the local patch clears to navy only.
  world_->GetCamera(&lon_, &lat_, &height_m_);
  SyncOrbitFromCamera(lon_, lat_, height_m_, &yaw_, &pitch_, &distance_);
  dx12_ = true;
  backend_ = "flycube-dx12";
  width_ = width;
  height_ = height;
  // Subclass so product pointer samples reach SceneImpl (DLL child has no
  // mouse routing of its own).
  SetWindowLongPtrW(child_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  SetWindowLongPtrW(
      child_, GWLP_WNDPROC,
      reinterpret_cast<LONG_PTR>(static_cast<WNDPROC>(FlyCubeHostProc)));
  return true;
}

void FlyCubeHost::DestroyWorld() {
  if (!world_)
    return;
  world_->Destroy();
  delete world_;
  world_ = NULL;
  child_ = NULL;
  dx12_ = false;
}

bool FlyCubeHost::PresentWorld() {
  return world_ && world_->Present();
}

bool FlyCubeHost::Create(HWND parent, int width, int height) {
  if (!parent || !IsWindow(parent))
    return false;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  Destroy();
  if (TryWorldCreate(parent, width, height)) {
    Present();
    return true;
  }
  if (!RegisterHostClass())
    return false;
  child_ = CreateWindowExW(0, kClassName, L"",
                           WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                               WS_CLIPCHILDREN,
                           0, 0, width, height, parent, NULL,
                           GetModuleHandleW(NULL), this);
  if (!child_)
    return false;
  width_ = width;
  height_ = height;
  EnsureMesh();
  mesh_ready_ = g_mesh;
  backend_ = "flycube-gl";
  Present();
  return child_ != NULL;
}

HWND FlyCubeHost::hwnd() const {
  return child_;
}

void FlyCubeHost::Destroy() {
  in_present_ = true;
  if (dx12_) {
    DestroyWorld();
    in_present_ = false;
    return;
  }
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
  mesh_ready_ = false;
  in_present_ = false;
}

void FlyCubeHost::Resize(int width, int height) {
  // NativeViewHost owns HWND placement (physical px). Match client rect so
  // DIP resize args cannot shrink a HIDPI child.
  if (child_ && IsWindow(child_)) {
    RECT client = {};
    if (GetClientRect(child_, &client)) {
      const int cw = client.right - client.left;
      const int ch = client.bottom - client.top;
      if (cw >= 2 && ch >= 2) {
        width = cw;
        height = ch;
      }
    }
  }
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  width_ = width;
  height_ = height;
  if (world_)
    world_->Resize(width, height);
}

void FlyCubeHost::Orbit(double dx_px, double dy_px) {
  if (world_) {
    world_->Orbit(dx_px, dy_px);
    world_->GetCamera(&lon_, &lat_, &height_m_);
    return;
  }
  lon_ += dx_px * 0.05;
  lat_ += dy_px * 0.05;
  if (lat_ > 85.0)
    lat_ = 85.0;
  if (lat_ < -85.0)
    lat_ = -85.0;
  SyncOrbitFromCamera(lon_, lat_, height_m_, &yaw_, &pitch_, &distance_);
}

void FlyCubeHost::Zoom(double wheel_dy) {
  height_m_ *= std::pow(0.9, wheel_dy);
  if (height_m_ < 50.0)
    height_m_ = 50.0;
  if (height_m_ > 2.0e7)
    height_m_ = 2.0e7;
  SyncOrbitFromCamera(lon_, lat_, height_m_, &yaw_, &pitch_, &distance_);
  if (world_)
    world_->SetCamera(lon_, lat_, height_m_);
}

void FlyCubeHost::Camera(double* lon, double* lat, double* height_m) const {
  if (world_) {
    double a = lon_, b = lat_, c = height_m_;
    world_->GetCamera(&a, &b, &c);
    if (lon)
      *lon = a;
    if (lat)
      *lat = b;
    if (height_m)
      *height_m = c;
    return;
  }
  if (lon)
    *lon = lon_;
  if (lat)
    *lat = lat_;
  if (height_m)
    *height_m = height_m_;
}

void FlyCubeHost::SetCamera(double lon, double lat, double height_m) {
  lon_ = lon;
  lat_ = lat;
  height_m_ = height_m;
  SyncOrbitFromCamera(lon_, lat_, height_m_, &yaw_, &pitch_, &distance_);
  if (world_)
    world_->SetCamera(lon_, lat_, height_m_);
}

void FlyCubeHost::SetPointerCallback(PointerFn fn, void* user) {
  pointer_fn_ = fn;
  pointer_user_ = user;
}

void FlyCubeHost::NotifyPointer(int kind,
                                int action,
                                double x,
                                double y,
                                double wheel_dx,
                                double wheel_dy) {
  if (in_present_ || !pointer_fn_)
    return;
  pointer_fn_(pointer_user_, kind, action, x, y, wheel_dx, wheel_dy);
}

bool FlyCubeHost::EnsureGl() {
  if (gl_)
    return true;
  dc_ = GetDC(child_);
  if (!dc_)
    return false;
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  const int format = ChoosePixelFormat(dc_, &pfd);
  if (format == 0 || !SetPixelFormat(dc_, format, &pfd))
    return false;
  gl_ = wglCreateContext(dc_);
  return gl_ != NULL;
}

bool FlyCubeHost::PresentGl() {
  if (in_present_ || !child_ || !mesh_ready_)
    return false;
  in_present_ = true;
  bool ok = false;
  if (EnsureGl() && wglMakeCurrent(dc_, gl_)) {
    while (glGetError() != GL_NO_ERROR) {
    }
    const float aspect = height_ > 0 ? static_cast<float>(width_) /
                                           static_cast<float>(height_)
                                     : 1.0f;
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    const float ambient[] = {0.28f, 0.28f, 0.30f, 1.0f};
    const float diffuse[] = {0.9f, 0.9f, 0.85f, 1.0f};
    const float position[] = {0.45f, 0.7f, 0.9f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glClearColor(0.05f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    Perspective(35.0f, aspect, 0.1f, 20.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -static_cast<float>(distance_));
    glRotatef(static_cast<float>(pitch_ * 180.0 / 3.14159265), 1.0f, 0.0f, 0.0f);
    glRotatef(static_cast<float>(yaw_ * 180.0 / 3.14159265), 0.0f, 1.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    const float* xyz = &g_verts[0].x;
    glVertexPointer(3, GL_FLOAT, sizeof(Vert), xyz);
    glNormalPointer(GL_FLOAT, sizeof(Vert), xyz);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vert), &g_verts[0].r);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_indices.size()),
                   GL_UNSIGNED_SHORT, g_indices.data());
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    std::vector<uint8_t> pixels(static_cast<size_t>(width_) * height_ * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    bool varies = false;
    const unsigned hash = HashGrid(pixels.data(), width_, height_, &varies);
    const GLenum error = glGetError();
    const uint8_t* sample = pixels.data();
    const bool clear_only = std::abs(static_cast<int>(sample[0]) - 13) <= 3 &&
                            std::abs(static_cast<int>(sample[1]) - 15) <= 3 &&
                            std::abs(static_cast<int>(sample[2]) - 23) <= 3;
    const bool empty = (sample[0] | sample[1] | sample[2]) == 0;
    if (error == GL_NO_ERROR && (varies || (!clear_only && !empty)) &&
        SwapBuffers(dc_)) {
      token_ = hash;
      ok = true;
    }
    wglMakeCurrent(NULL, NULL);
  }
  in_present_ = false;
  return ok;
}

bool FlyCubeHost::Present() {
  if (dx12_)
    return PresentWorld();
  return PresentGl();
}

}  // namespace content
