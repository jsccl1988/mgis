// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/present/ue_presenter.h"

#include <windows.h>

#include "content/browser/ipc/pipe_client.h"
#include "content/browser/ipc/pipe_message.h"
#include "content/public/browser/trace_log.h"

namespace content {
namespace {

const wchar_t kClassName[] = L"MViewUePresenter";
bool g_class_ready = false;

LRESULT CALLBACK UeProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_PAINT) {
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(hwnd, &paint);
    RECT rect;
    GetClientRect(hwnd, &rect);
    FillRect(dc, &rect, reinterpret_cast<HBRUSH>(COLOR_HIGHLIGHT + 1));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    DrawTextW(dc, L"UE peer (pipe connected)", -1, &rect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    EndPaint(hwnd, &paint);
    return 0;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

bool EnsureClass() {
  if (g_class_ready)
    return true;
  WNDCLASSW wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.lpfnWndProc = UeProc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    return false;
  g_class_ready = true;
  return true;
}

}  // namespace

UePresenter::UePresenter()
    : child_(NULL),
      width_(0),
      height_(0),
      lon_(121.47),
      lat_(31.23),
      height_m_(2000.0),
      orbit_delta_(0) {}

UePresenter::~UePresenter() {
  if (child_)
    DestroyWindow(reinterpret_cast<HWND>(child_));
  child_ = NULL;
  if (pipe_)
    pipe_->close();
}

bool UePresenter::TryConnect(TraceLog* trace) {
  pipe_.reset(new PipeClient());
  std::string error;
  if (!pipe_->connect(&error)) {
    if (trace)
      trace->Append(kTraceConsole, "presenter.ue_failed");
    pipe_.reset();
    return false;
  }
  return true;
}

bool UePresenter::Create(void* parent_hwnd,
                         int width,
                         int height,
                         TraceLog* trace) {
  if (!parent_hwnd || !pipe_ || !pipe_->connected() || !EnsureClass())
    return false;
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;
  HWND child = CreateWindowExW(
      0, kClassName, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, width_,
      height_, reinterpret_cast<HWND>(parent_hwnd), NULL, GetModuleHandleW(NULL),
      NULL);
  if (!child)
    return false;
  child_ = child;
  if (trace)
    trace->Append(kTraceConsole, "presenter.3d ue");
  Present();
  return true;
}

void* UePresenter::hwnd() const {
  return child_;
}

void UePresenter::Resize(int width, int height) {
  if (!child_)
    return;
  width_ = width < 2 ? 2 : width;
  height_ = height < 2 ? 2 : height;
  // NativeViewHost owns screen position. MoveWindow(..., 0, 0, ...) resets the
  // origin to the parent client corner and covers workbench chrome.
  HWND hwnd = reinterpret_cast<HWND>(child_);
  if (IsWindow(hwnd)) {
    SetWindowPos(hwnd, NULL, 0, 0, width_, height_,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }
}

void UePresenter::Present() {
  if (child_)
    InvalidateRect(reinterpret_cast<HWND>(child_), NULL, TRUE);
}

void UePresenter::Orbit(double dx_px, double dy_px) {
  if (dx_px != 0.0 || dy_px != 0.0)
    orbit_delta_ = 1.0;
  lon_ += dx_px * 0.05;
  lat_ += dy_px * 0.05;
  SendCameraSet();
  Present();
}

void UePresenter::Camera(double* lon, double* lat, double* height) const {
  if (lon)
    *lon = lon_;
  if (lat)
    *lat = lat_;
  if (height)
    *height = height_m_;
}

void UePresenter::SetCamera(double lon, double lat, double height) {
  lon_ = lon;
  lat_ = lat;
  height_m_ = height;
  SendCameraSet();
  Present();
}

void UePresenter::SendCameraSet() {
  if (!pipe_ || !pipe_->connected())
    return;
  PipeMessage request;
  request.v = 1;
  request.id = "scene-cam";
  request.op = "camera.set";
  request.body.SetObject();
  rapidjson::Document::AllocatorType& alloc = request.body.GetAllocator();
  request.body.AddMember("view", rapidjson::Value("view0", alloc), alloc);
  request.body.AddMember("origin", rapidjson::Value("scene", alloc), alloc);
  request.body.AddMember("seq", 1, alloc);
  request.body.AddMember("longitude", lon_, alloc);
  request.body.AddMember("latitude", lat_, alloc);
  request.body.AddMember("height", height_m_, alloc);
  request.body.AddMember("heading", 0.0, alloc);
  request.body.AddMember("pitch", 0.0, alloc);
  PipeMessage response;
  std::string error;
  (void)pipe_->transact(request, &response, &error);
}

std::string UePresenter::BackendName() const {
  return "ue";
}

}  // namespace content
