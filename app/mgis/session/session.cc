// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/session/session.h"

#include <string>
#include <vector>

#include "app/mgis/horizon/tool_plugin_panel.h"
#include "app/mgis/viewport/viewport_surface.h"
#include "base/logging.h"
#include "content/public/browser/plugin_host.h"
#include "content/public/browser/scene_mode.h"
#include "content/public/browser/types.h"
#include "ui/views/controls/native/native_view_host.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace mgis {
namespace {

std::wstring ModulePath() {
  std::wstring buf(32768, L'\0');
  DWORD n = GetModuleFileNameW(NULL, &buf[0], static_cast<DWORD>(buf.size()));
  if (n == 0 || n >= buf.size())
    return std::wstring();
  buf.resize(n);
  return buf;
}

std::wstring ParentDir(const std::wstring& path) {
  const size_t slash = path.find_last_of(L"\\/");
  if (slash == std::wstring::npos || slash == 0)
    return std::wstring();
  return path.substr(0, slash);
}

bool FileExists(const std::wstring& path) {
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool IsRepoRoot(const std::wstring& dir) {
  return FileExists(dir + L"\\BUILD.gn") &&
         FileExists(dir + L"\\plugin\\sample\\plugin.json");
}

std::wstring FindRepoRootFrom(std::wstring dir) {
  for (int i = 0; i < 8 && !dir.empty(); ++i) {
    if (IsRepoRoot(dir))
      return dir;
    const std::wstring parent = ParentDir(dir);
    if (parent.empty() || parent == dir)
      break;
    dir = parent;
  }
  return std::wstring();
}

std::string WideToUtf8(const std::wstring& wide) {
  if (wide.empty())
    return std::string();
  const int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                    static_cast<int>(wide.size()), NULL, 0,
                                    NULL, NULL);
  if (n <= 0)
    return std::string();
  std::string out(static_cast<size_t>(n), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                      &out[0], n, NULL, NULL);
  return out;
}

void HideSceneChild(void* child_hwnd) {
  if (!child_hwnd)
    return;
  HWND hwnd = reinterpret_cast<HWND>(child_hwnd);
  if (IsWindow(hwnd))
    ShowWindow(hwnd, SW_HIDE);
}

// NativeViewHost::Layout sizes/shows via GetVisibleBounds. After tab switch the
// host may still report empty bounds until ViewportSurface::Layout runs; force
// both, then raise the HWND above the Skia fill (Layout uses SWP_NOZORDER).
void ShowAttachedChild(ViewportSurface* surface, void* child_hwnd) {
  if (!surface || !surface->native_host() || !child_hwnd)
    return;
  surface->Layout();
  views::NativeViewHost* host = surface->native_host();
  // Layout may HideWidget when GetVisibleBounds is still empty right after a
  // tab switch; raise and show again so DX12 China DEM is not left SW_HIDE.
  host->Layout();
  HWND hwnd = reinterpret_cast<HWND>(child_hwnd);
  if (!IsWindow(hwnd))
    return;
  SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
  ShowWindow(hwnd, SW_SHOWNA);
}

void AttachOrDetach(ViewportSurface* surface, void* child_hwnd, bool attach) {
  if (!surface || !surface->native_host())
    return;
  views::NativeViewHost* host = surface->native_host();
  if (attach && child_hwnd) {
    gfx::NativeView native =
        reinterpret_cast<gfx::NativeView>(child_hwnd);
    // StartWorkbenchSession and ApplySceneMode both reparent; Attach is not
    // idempotent (DCHECK(!native_view_)).
    if (host->native_view() == native) {
      ShowAttachedChild(surface, child_hwnd);
      return;
    }
    if (host->native_view())
      host->Detach();
    host->Attach(native);
    ShowAttachedChild(surface, child_hwnd);
  } else {
    if (host->native_view())
      host->Detach();
    // Presenters create both children as WS_VISIBLE under the widget HWND.
    // The inactive child is never Attached on first reparent, so Detach alone
    // would leave FlyCube/MapLibre covering chrome — hide it explicitly.
    HideSceneChild(child_hwnd);
  }
}

void SyncActiveSurfaceSize(content::GisContents* contents,
                           ViewportSurface* map_page,
                           ViewportSurface* view0_page) {
  if (!contents)
    return;
  const content::SceneMode mode = contents->scene_mode();
  const bool map_active = mode == content::SceneMode::kScene2D ||
                          mode == content::SceneMode::kColumbusView;
  ViewportSurface* active = map_active ? map_page : view0_page;
  ViewportSurface* inactive = map_active ? view0_page : map_page;
  if (!active)
    return;
  int width = active->width();
  int height = active->height();
  // Inactive TabbedPane pages keep size from TabbedPane::Layout; if the active
  // page was just selected and still reports 0x0, borrow the sibling size.
  if ((width < 2 || height < 2) && inactive) {
    if (width < 2)
      width = inactive->width();
    if (height < 2)
      height = inactive->height();
  }
  if (width < 2 || height < 2)
    return;
  contents->ResizeScene(width, height);
  // ResizeScene only updates the active presenter swapchain/GL; NativeViewHost
  // must Layout again so ShowWidget matches the new size (DX12 MoveWindow must
  // not reset origin — presenters use SWP_NOMOVE).
  ShowAttachedChild(active, contents->scene_child_hwnd());
  contents->PresentScene();
}

}  // namespace

void ReparentSceneSurfaces(content::GisContents* contents,
                           ViewportSurface* map_page,
                           ViewportSurface* view0_page) {
  if (!contents)
    return;
  const content::SceneMode mode = contents->scene_mode();
  void* map_child =
      contents->scene_child_hwnd_for(content::SceneMode::kScene2D);
  void* view0_child =
      contents->scene_child_hwnd_for(content::SceneMode::kScene3D);
  const bool map_active = mode == content::SceneMode::kScene2D ||
                          mode == content::SceneMode::kColumbusView;
  // Both presenters create WS_VISIBLE children under the widget. Hide first so
  // a never-Attached inactive HWND cannot cover chrome; Attach shows the
  // active one via NativeViewHost.
  HideSceneChild(map_child);
  HideSceneChild(view0_child);
  AttachOrDetach(map_page, map_child, map_active);
  AttachOrDetach(view0_page, view0_child, !map_active);
  SyncActiveSurfaceSize(contents, map_page, view0_page);
}

bool StartWorkbenchSession(content::GisContents* contents,
                           content::TraceLog::Observer* trace_observer,
                           ViewportSurface* map_page,
                           ViewportSurface* view0_page,
                           void* parent_hwnd,
                           int width,
                           int height) {
  if (!contents)
    return false;

  contents->trace_log()->AddObserver(trace_observer);

  if (width < 2)
    width = 640;
  if (height < 2)
    height = 480;

  if (!parent_hwnd) {
    LOG(ERROR) << "mgis session: missing parent hwnd";
    return false;
  }

  if (!contents->AttachView(parent_hwnd, content::kViewIdScene, width,
                            height)) {
    LOG(ERROR) << "mgis session: AttachView failed";
    return false;
  }

  ReparentSceneSurfaces(contents, map_page, view0_page);

  contents->trace_log()->Append(content::kTraceConsole, "session.ready");
  LOG(INFO) << "mgis content session started";
  return true;
}

bool LoadWorkbenchPlugins(content::GisContents* contents,
                          plugin::Registry* registry,
                          plugin::Loader* loader,
                          ToolPluginPanel* panel) {
  if (!contents || !registry || !loader)
    return false;

  const std::wstring exe_dir = ParentDir(ModulePath());
  const std::wstring repo = FindRepoRootFrom(exe_dir);

  std::vector<std::string> scan_dirs;
  if (!exe_dir.empty()) {
    scan_dirs.push_back(WideToUtf8(exe_dir + L"\\UI\\plugins"));
    scan_dirs.push_back(WideToUtf8(exe_dir + L"\\plugins"));
  }
  if (!repo.empty())
    scan_dirs.push_back(WideToUtf8(repo + L"\\plugin\\sample"));

  bool scanned = false;
  for (size_t i = 0; i < scan_dirs.size(); ++i) {
    if (scan_dirs[i].empty())
      continue;
    if (registry->Scan(scan_dirs[i])) {
      scanned = true;
      break;
    }
  }
  if (!scanned) {
    LOG(WARNING) << "mgis session: no plugin directory scanned";
    if (panel)
      panel->SetPluginHost(contents->plugin_host());
    return false;
  }

  std::vector<std::string> dll_dirs;
  if (!exe_dir.empty())
    dll_dirs.push_back(WideToUtf8(exe_dir));
  if (!repo.empty()) {
    dll_dirs.push_back(WideToUtf8(repo + L"\\out\\Debug"));
    dll_dirs.push_back(WideToUtf8(repo + L"\\out\\Release"));
  }

  const int loaded = loader->LoadAll(*registry, contents->plugin_host(),
                                     dll_dirs, contents->trace_log());
  LOG(INFO) << "mgis session: loaded " << loaded << " plugin dll(s)";

  if (panel)
    panel->SetPluginHost(contents->plugin_host());
  return loaded > 0;
}

}  // namespace mgis
