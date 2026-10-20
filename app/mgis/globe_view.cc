// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/globe_view.h"

#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

const int kKindMouse = 0;
const int kKindWheel = 1;
const int kActionMove = 1;
const int kActionPress = 2;
const int kActionRelease = 3;

const int kStacks = 18;
const int kSlices = 36;
const wchar_t kClassName[] = L"MViewGlobeView";

struct Vert {
  float x, y, z;
  float u, v;
  unsigned char r, g, b, a;
};

std::vector<Vert> g_verts;
std::vector<unsigned short> g_indices;
bool g_mesh = false;

bool Land(float lon_deg, float lat_deg) {
  if (lat_deg > 72.0f || lat_deg < -56.0f)
    return false;
  if (lon_deg > -130.0f && lon_deg < -60.0f && lat_deg > 25.0f && lat_deg < 50.0f)
    return true;
  if (lon_deg > -10.0f && lon_deg < 30.0f && lat_deg > 35.0f && lat_deg < 60.0f)
    return true;
  if (lon_deg > 70.0f && lon_deg < 140.0f && lat_deg > 10.0f && lat_deg < 50.0f)
    return true;
  if (lon_deg > -20.0f && lon_deg < 50.0f && lat_deg > -35.0f && lat_deg < 35.0f)
    return true;
  if (lon_deg > 110.0f && lon_deg < 155.0f && lat_deg > -45.0f && lat_deg < -10.0f)
    return true;
  if (lon_deg > -80.0f && lon_deg < -35.0f && lat_deg > -55.0f && lat_deg < 12.0f)
    return true;
  return false;
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
      const bool ice = lat_deg > 72.0f || lat_deg < -62.0f;
      const bool land = Land(lon_deg, lat_deg);
      if (ice) {
        vert.r = 230;
        vert.g = 236;
        vert.b = 242;
      } else if (land) {
        vert.r = 126;
        vert.g = 168;
        vert.b = 78;
      } else {
        vert.r = 48;
        vert.g = 110;
        vert.b = 168;
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

bool RegisterGlobeClass() {
  static bool registered = false;
  if (registered)
    return true;
  WNDCLASSW wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = GlobeViewProc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  registered = RegisterClassW(&wc) != 0 ||
               GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
  return registered;
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

}  // namespace

LRESULT CALLBACK GlobeViewProc(HWND hwnd,
                               UINT msg,
                               WPARAM wparam,
                               LPARAM lparam) {
  if (msg == WM_NCCREATE) {
    CREATESTRUCTW* created = reinterpret_cast<CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(created->lpCreateParams));
  }
  GlobeView* view =
      reinterpret_cast<GlobeView*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (msg) {
    case WM_ERASEBKGND:
      return 1;
    case WM_LBUTTONDOWN:
      SetCapture(hwnd);
      SetFocus(hwnd);
      if (view)
        view->Notify(kKindMouse, kActionPress,
           static_cast<double>(static_cast<short>(LOWORD(lparam))),
           static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      return 0;
    case WM_MOUSEMOVE:
      if (view && GetCapture() == hwnd) {
        view->Notify(kKindMouse, kActionMove,
             static_cast<double>(static_cast<short>(LOWORD(lparam))),
             static_cast<double>(static_cast<short>(HIWORD(lparam))), 0.0, 0.0);
      }
      return 0;
    case WM_LBUTTONUP:
      if (GetCapture() == hwnd)
        ReleaseCapture();
      if (view)
        view->Notify(kKindMouse, kActionRelease,
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
      if (view)
        view->Notify(kKindWheel, kActionMove, static_cast<double>(pt.x),
                     static_cast<double>(pt.y), 0.0, wheel);
      return 0;
    }
    default:
      break;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

GlobeView::GlobeView()
    : child_(NULL),
      dc_(NULL),
      gl_(NULL),
      in_present_(false),
      mesh_ready_(false),
      yaw_(0.4),
      pitch_(0.3),
      distance_(3.2),
      width_(0),
      height_(0),
      token_(0),
      draw_calls_(0),
      callback_(NULL),
      user_(NULL) {}

GlobeView::~GlobeView() {
  in_present_ = true;
  callback_ = NULL;
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

bool GlobeView::Attach(HWND parent, int width, int height, double yaw_radians) {
  if (!parent || !IsWindow(parent) || !RegisterGlobeClass())
    return false;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  yaw_ = yaw_radians;
  if (!child_) {
    child_ = CreateWindowExW(0, kClassName, L"",
                             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                 WS_CLIPCHILDREN,
                             0, 0, width, height, parent, NULL,
                             GetModuleHandleW(NULL), this);
  }
  if (!child_)
    return false;
  width_ = width;
  height_ = height;
  EnsureMesh();
  mesh_ready_ = g_mesh;
  return true;
}

void GlobeView::Resize(int width, int height) {
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

void GlobeView::SetDrawSize(int width, int height) {
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  width_ = width;
  height_ = height;
}

void GlobeView::Notify(int kind,
                       int action,
                       double x,
                       double y,
                       double wheel_dx,
                       double wheel_dy) {
  if (in_present_ || !callback_)
    return;
  callback_(user_, kind, action, x, y, wheel_dx, wheel_dy);
}

void GlobeView::SetPointerCallback(GlobePointerFn fn, void* user) {
  callback_ = fn;
  user_ = user;
}

void GlobeView::Orbit(double dx_px, double dy_px) {
  yaw_ += dx_px * 0.01;
  pitch_ += dy_px * 0.01;
  if (pitch_ > 1.2)
    pitch_ = 1.2;
  if (pitch_ < -1.2)
    pitch_ = -1.2;
}

void GlobeView::Zoom(double wheel_dy) {
  distance_ *= std::pow(0.9, wheel_dy);
  if (distance_ < 1.4)
    distance_ = 1.4;
  if (distance_ > 8.0)
    distance_ = 8.0;
}

bool GlobeView::EnsureGl() {
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

bool GlobeView::Present() {
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
    draw_calls_ = 1;
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    std::vector<uint8_t> pixels(static_cast<size_t>(width_) * height_ * 3);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    bool varies = false;
    const unsigned hash = HashGrid(pixels.data(), width_, height_, &varies);
    const GLenum error = glGetError();
    if (varies && error == GL_NO_ERROR && SwapBuffers(dc_)) {
      token_ = hash;
      ok = true;
    }
    wglMakeCurrent(NULL, NULL);
  }
  in_present_ = false;
  return ok;
}
