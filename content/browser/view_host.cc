// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/view_host.h"

#include <windows.h>

#include "base/logging.h"

namespace content {
namespace {

const wchar_t kPlaceholderClass[] = L"MViewFlyCubeHost";

ViewBackendHooks g_hooks[4];
ViewHost* g_attaching = NULL;
bool g_class_ready = false;

ViewBackendHooks HooksFor(ViewBackend backend) {
  if (backend < kViewBackendFlyCube || backend > kViewBackendUe)
    return ViewBackendHooks();
  return g_hooks[backend];
}

void Note(TraceLog* trace, const std::string& text) {
  LOG(INFO) << text;
  if (trace)
    trace->Append(kTraceConsole, text);
}

LRESULT CALLBACK PlaceholderProc(HWND hwnd,
                                 UINT msg,
                                 WPARAM wparam,
                                 LPARAM lparam) {
  ViewHost* host = reinterpret_cast<ViewHost*>(
      GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (msg == WM_NCCREATE) {
    CREATESTRUCTW* created = reinterpret_cast<CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(created->lpCreateParams));
    return DefWindowProcW(hwnd, msg, wparam, lparam);
  }
  if (!host)
    return DefWindowProcW(hwnd, msg, wparam, lparam);

  InputEvent event;
  event.view_id = host->view_id();
  event.x = static_cast<int>(static_cast<short>(LOWORD(lparam)));
  event.y = static_cast<int>(static_cast<short>(HIWORD(lparam)));
  bool gesture = false;
  switch (msg) {
    case WM_MOUSEMOVE:
      event.kind = kInputEventMouse;
      event.mouse_action = kMouseMove;
      gesture = true;
      break;
    case WM_LBUTTONDOWN:
      event.kind = kInputEventMouse;
      event.mouse_action = kMouseDown;
      event.button = 1;
      gesture = true;
      break;
    case WM_LBUTTONUP:
      event.kind = kInputEventMouse;
      event.mouse_action = kMouseUp;
      event.button = 1;
      gesture = true;
      break;
    case WM_MOUSEWHEEL: {
      POINT pt;
      pt.x = static_cast<short>(LOWORD(lparam));
      pt.y = static_cast<short>(HIWORD(lparam));
      ScreenToClient(hwnd, &pt);
      event.kind = kInputEventWheel;
      event.x = pt.x;
      event.y = pt.y;
      event.wheel_delta = static_cast<int>(static_cast<short>(HIWORD(wparam)));
      gesture = true;
      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC dc = BeginPaint(hwnd, &paint);
      RECT rect;
      GetClientRect(hwnd, &rect);
      FillRect(dc, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
      DrawTextW(dc, L"Cesium / FlyCube", -1, &rect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      EndPaint(hwnd, &paint);
      return 0;
    }
    default:
      break;
  }
  if (gesture)
    host->NotifyGesture(event);
  if (gesture)
    return 0;
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

bool EnsurePlaceholderClass() {
  if (g_class_ready)
    return true;
  WNDCLASSW wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.lpfnWndProc = PlaceholderProc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = kPlaceholderClass;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    return false;
  g_class_ready = true;
  return true;
}

void* CreatePlaceholder(void* parent, int width, int height, ViewHost* host) {
  if (!EnsurePlaceholderClass())
    return NULL;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  return CreateWindowExW(0, kPlaceholderClass, L"",
                         WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, width,
                         height, reinterpret_cast<HWND>(parent), NULL,
                         GetModuleHandleW(NULL), host);
}

}  // namespace

ViewBackendHooks::ViewBackendHooks()
    : create(NULL),
      resize(NULL),
      destroy(NULL),
      present(NULL),
      user(NULL) {}

ViewHost::Delegate::Delegate() {}

ViewHost::Delegate::~Delegate() {}

ViewHost::ViewHost()
    : delegate_(NULL),
      trace_(NULL),
      child_(NULL),
      backend_(kViewBackendFlyCube),
      factory_owned_(false) {}

ViewHost::~ViewHost() {
  Detach();
}

void ViewHost::set_delegate(Delegate* delegate) {
  delegate_ = delegate;
}

void ViewHost::set_trace(TraceLog* trace) {
  trace_ = trace;
}

void ViewHost::SetBackendHooks(ViewBackend backend,
                               const ViewBackendHooks& hooks) {
  if (backend < kViewBackendFlyCube || backend > kViewBackendUe)
    return;
  g_hooks[backend] = hooks;
}

ViewHost* ViewHost::AttachingHost() {
  return g_attaching;
}

bool ViewHost::Attach(void* parent_hwnd,
                      const std::string& view_id,
                      ViewBackend backend,
                      int width,
                      int height) {
  Detach();
  if (!parent_hwnd || !is_known_view_id(view_id)) {
    Note(trace_, "view.attach rejected");
    return false;
  }
  if (backend == kViewBackendMapLibre || backend == kViewBackendUe) {
    const char* name = backend == kViewBackendMapLibre ? "maplibre" : "ue";
    Note(trace_, std::string("backend.unavailable ") + name);
    return false;
  }
  if (backend != kViewBackendFlyCube)
    return false;

  TraceScope scope(trace_, "view.attach.flycube");
  view_id_ = view_id;
  backend_ = backend;
  const ViewBackendHooks hooks = HooksFor(backend);
  g_attaching = this;
  if (hooks.create) {
    child_ = hooks.create(parent_hwnd, width, height, hooks.user);
    factory_owned_ = child_ != NULL;
  } else {
    child_ = CreatePlaceholder(parent_hwnd, width, height, this);
    factory_owned_ = false;
  }
  g_attaching = NULL;
  if (!child_) {
    Note(trace_, "flycube.attach failed");
    return false;
  }
  if (hooks.present)
    hooks.present(child_, hooks.user);
  Note(trace_, std::string("flycube.attached ") + view_id);
  return true;
}

void ViewHost::Resize(int width, int height) {
  if (!child_)
    return;
  if (width < 1)
    width = 1;
  if (height < 1)
    height = 1;
  const ViewBackendHooks hooks = HooksFor(backend_);
  if (hooks.resize)
    hooks.resize(child_, width, height, hooks.user);
}

void ViewHost::Present() {
  if (!child_)
    return;
  TraceScope scope(trace_, "view.present");
  const ViewBackendHooks hooks = HooksFor(backend_);
  if (factory_owned_ && hooks.present)
    hooks.present(child_, hooks.user);
}

void ViewHost::Detach() {
  if (!child_)
    return;
  void* child = child_;
  child_ = NULL;
  const ViewBackendHooks hooks = HooksFor(backend_);
  if (factory_owned_ && hooks.destroy)
    hooks.destroy(child, hooks.user);
  else if (!factory_owned_)
    DestroyWindow(reinterpret_cast<HWND>(child));
  factory_owned_ = false;
}

bool ViewHost::NotifyGesture(const InputEvent& event) {
  if (!delegate_)
    return false;
  return delegate_->OnChildGesture(event);
}

void* ViewHost::child_hwnd() const {
  return child_;
}

ViewBackend ViewHost::backend() const {
  return backend_;
}

const std::string& ViewHost::view_id() const {
  return view_id_;
}

}  // namespace content
