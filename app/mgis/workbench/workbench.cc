// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/workbench/workbench.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "app/mgis/horizon/bottom_panel.h"
#include "app/mgis/horizon/status_bar.h"
#include "app/mgis/horizon/tool_plugin_panel.h"
#include "app/mgis/session/session.h"
#include "app/mgis/viewport/viewport_surface.h"
#include "app/mgis/workbench/workbench_markup.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/time.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/gis_contents.h"
#include "content/public/browser/product_presenters.h"
#include "content/public/browser/trace_log.h"
#include "content/public/browser/types.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/menu/menu.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/markup/loader/markup_loader.h"
#include "ui/views/widget/widget.h"
#include "ui/views/win/hwnd_util.h"

namespace mgis {
namespace {

bool g_quit_when_main_closes = true;

// NativeWidgetWin / HWNDMessageHandler layout in physical pixels. Design
// constants in this file are 96-DPI DIP; convert with the window DPI when
// known, else the process device scale (EnableHighDPISupport).
float WorkbenchDpiScale(views::Widget* widget) {
  HWND hwnd = widget ? views::HWNDForWidget(widget) : NULL;
  if (hwnd && IsWindow(hwnd)) {
    typedef UINT(WINAPI *GetDpiForWindowFn)(HWND);
    GetDpiForWindowFn get_dpi_for_window = reinterpret_cast<GetDpiForWindowFn>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    if (get_dpi_for_window) {
      const UINT window_dpi = get_dpi_for_window(hwnd);
      if (window_dpi >= 96)
        return static_cast<float>(window_dpi) / 96.0f;
    }
  }
  const float scale = ui::win::GetDeviceScaleFactor();
  return scale > 0.0f ? scale : 1.0f;
}

int ScaleDip(int dip, float scale) {
  return static_cast<int>(dip * scale + 0.5f);
}

void ForceWorkbenchBounds(views::Widget* widget, const gfx::Rect& dip_bounds) {
  if (!widget)
    return;
  // Design rect is DIP (1100x720). Scale before SetBounds so first layout is
  // not crushed on 150%/250% monitors (GetDpiForWindow may still be 96 before
  // Show; GetDeviceScaleFactor covers that case).
  const float scale = WorkbenchDpiScale(widget);
  const gfx::Rect physical(
      ScaleDip(dip_bounds.x(), scale), ScaleDip(dip_bounds.y(), scale),
      ScaleDip(dip_bounds.width(), scale), ScaleDip(dip_bounds.height(), scale));
  HWND hwnd = views::HWNDForWidget(widget);
  if (hwnd && IsWindow(hwnd)) {
    SetWindowPos(hwnd, NULL, physical.x(), physical.y(), physical.width(),
                 physical.height(), SWP_NOZORDER | SWP_NOACTIVATE);
  }
  widget->SetBounds(physical);
  if (widget->GetContentsView())
    widget->GetContentsView()->Layout();
}

}  // namespace

class WorkbenchContents::WindowMenuDelegate : public views::Menu::Delegate {
 public:
  WindowMenuDelegate() {}
  virtual ~WindowMenuDelegate() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(WindowMenuDelegate);
};

WorkbenchContents::WorkbenchContents()
    : active_(kMap),
      syncing_(false),
      chrome_root_(NULL),
      toolbar_(NULL),
      catalog_(NULL),
      view_tabs_(NULL),
      map_page_(NULL),
      view0_page_(NULL),
      tools_panel_(NULL),
      bottom_(NULL),
      status_(NULL) {
  menu_delegate_.reset(new WindowMenuDelegate());

  toolbar_ = new MapToolbar();
  toolbar_->set_delegate(this);

  catalog_ = new CatalogTabs();
  catalog_->set_observer(this);

  map_page_ = new ViewportSurface();
  view0_page_ = new ViewportSurface();

  view_tabs_ = new views::TabbedPane();
  view_tabs_->set_listener(this);
  view_tabs_->AddTab(ASCIIToUTF16("Map"), map_page_);
  view_tabs_->AddTab(ASCIIToUTF16("3D"), view0_page_);

  tools_panel_ = new ToolPluginPanel();
  bottom_ = new BottomPanel();
  status_ = new StatusBar();

  if (!InflateChromeFromMarkup()) {
    LOG(ERROR) << "workbench markup inflate failed; chrome will be empty";
    return;
  }
  MountSlot("toolbar_slot", toolbar_);
  MountSlot("catalog_slot", catalog_);
  MountSlot("viewport_slot", view_tabs_);
  MountSlot("tools_slot", tools_panel_);
  MountSlot("bottom_slot", bottom_);
  MountSlot("status_slot", status_);
}

WorkbenchContents::~WorkbenchContents() {
  debug_agent_.Stop();
  debug_agent_.set_contents(NULL);
  if (contents_.get()) {
    contents_->trace_log()->RemoveObserver(this);
    contents_->event_bus()->remove_observer(this);
  }
  if (map_page_ && map_page_->native_host())
    map_page_->native_host()->Detach();
  if (view0_page_ && view0_page_->native_host())
    view0_page_->native_host()->Detach();
}

bool WorkbenchContents::InflateChromeFromMarkup() {
  views::MarkupOptions options;
  // Prefer on-disk asset (UiDesigner / hot edit); fall back to embedded XML.
  if (!views::LoadMarkupInto("workbench.ui.xml", options, &markup_) ||
      !markup_.ok() || !markup_.root.get()) {
    const std::string_view embedded = WorkbenchMarkupXml();
    if (!views::LoadMarkupBytes(embedded, "", options, &markup_) ||
        !markup_.ok() || !markup_.root.get()) {
      LOG(ERROR) << "workbench markup: "
                 << (markup_.error.empty() ? "unknown" : markup_.error);
      return false;
    }
    LOG(INFO) << "workbench chrome from embedded markup";
  } else {
    LOG(INFO) << "workbench chrome from workbench.ui.xml";
  }

  chrome_root_ = markup_.root.release();
  AddChildView(chrome_root_);
  return chrome_root_ != NULL;
}

void WorkbenchContents::MountSlot(std::string_view slot_id, views::View* child) {
  if (!child)
    return;
  views::View* slot = markup_.ids.Find(slot_id);
  if (!slot) {
    LOG(ERROR) << "workbench slot missing: " << std::string(slot_id);
    return;
  }
  if (!slot->GetLayoutManager())
    slot->SetLayoutManager(new views::FillLayout());
  slot->AddChildView(child);
}

void WorkbenchContents::SelectViewTab(ViewId id) {
  if (id != kMap && id != kView0)
    id = kMap;
  if (syncing_)
    return;
  if (active_ == id && view_tabs_ &&
      view_tabs_->selected_tab_index() == static_cast<int>(id)) {
    return;
  }
  ApplyViewTab(id);
}

void WorkbenchContents::ApplyViewTab(ViewId id) {
  syncing_ = true;
  active_ = id;
  const int index = static_cast<int>(id);
  if (view_tabs_)
    view_tabs_->SelectTabAt(index);
  syncing_ = false;
  ApplySceneMode();
  UpdateStatusMessage();
}

void WorkbenchContents::ApplySceneMode() {
  if (!contents_.get())
    return;
  if (view_tabs_) {
    const int want = active_ == kView0 ? 1 : 0;
    if (view_tabs_->selected_tab_index() != want) {
      syncing_ = true;
      view_tabs_->SelectTabAt(want);
      syncing_ = false;
    }
  }
  if (map_page_)
    map_page_->Layout();
  if (view0_page_)
    view0_page_->Layout();
  const content::SceneMode mode = active_ == kView0
                                      ? content::SceneMode::kScene3D
                                      : content::SceneMode::kScene2D;
  contents_->SetSceneMode(mode);
  contents_->ActivateSceneTool(mode == content::SceneMode::kScene3D
                                   ? "view3d.trackball"
                                   : "view.pan");
  ReparentSceneSurfaces(contents_.get(), map_page_, view0_page_);
}

void WorkbenchContents::UpdateStatusMessage() {
  if (!status_)
    return;
  if (active_ == kView0)
    status_->SetMessage("3D scene present");
  else
    status_->SetMessage("2D map GPU present");
  status_->SetCrsText("EPSG:4326");
}

bool WorkbenchContents::HasLayerTree() const {
  return catalog_ && catalog_->layer_tree() != NULL;
}

bool WorkbenchContents::HasEntityTree() const {
  return catalog_ && catalog_->entity_tree() != NULL;
}

void WorkbenchContents::EnsureWindowMenu() {
  if (menu_)
    return;
  views::Widget* widget = GetWidget();
  if (!widget || !widget->GetNativeView())
    return;
  menu_.reset(views::Menu::Create(menu_delegate_.get(),
                                  views::Menu::TOPLEFT,
                                  widget->GetNativeView()));
  if (!menu_)
    return;
  menu_->AppendMenuItemWithLabel(1, ASCIIToUTF16("File"));
  menu_->AppendMenuItemWithLabel(2, ASCIIToUTF16("Edit"));
  menu_->AppendMenuItemWithLabel(3, ASCIIToUTF16("View"));
  menu_->AppendMenuItemWithLabel(4, ASCIIToUTF16("Layer"));
  menu_->AppendMenuItemWithLabel(5, ASCIIToUTF16("Plugins"));
}

bool WorkbenchContents::CanResize() const { return true; }
bool WorkbenchContents::CanMaximize() const { return true; }

string16 WorkbenchContents::GetWindowTitle() const {
  return ASCIIToUTF16("MView");
}

views::View* WorkbenchContents::GetContentsView() { return this; }

gfx::Size WorkbenchContents::GetPreferredSize() {
  // Match CreateWindow / ForceWorkbenchBounds: preferred size is physical px.
  return ui::win::DIPToScreenSize(gfx::Size(1100, 720));
}

void WorkbenchContents::LayoutSceneSurfaces() {
  if (!contents_.get())
    return;
  // Hide inactive WS_VISIBLE scene HWND, Attach the active page host, then
  // Resize/Present. Do not Resize using the full TabbedPane height — that
  // grows the child into the bottom rail and tools (overlay residual).
  ReparentSceneSurfaces(contents_.get(), map_page_, view0_page_);
}

void WorkbenchContents::Layout() {
  // Zone bounds come from Yoga (workbench.ui.xml). This only sizes the markup
  // root to the client area and drives scene HWND / Present.
  if (chrome_root_) {
    chrome_root_->SetBounds(0, 0, width(), height());
    chrome_root_->Layout();
  }
  LayoutSceneSurfaces();
}

void WorkbenchContents::WindowClosing() {
  debug_agent_.Stop();
  debug_agent_.set_contents(NULL);
  if (g_quit_when_main_closes && MessageLoopForUI::current())
    MessageLoopForUI::current()->Quit();
}

void WorkbenchContents::TabSelectedAt(int index) {
  if (syncing_)
    return;
  SelectViewTab(index == 0 ? kMap : kView0);
}

void WorkbenchContents::OnCatalogTabSelected(int /*index*/) {
  // Catalog tabs are independent of Map|3D (SmartGIS chrome).
}

void WorkbenchContents::OnMapToolSelected(const std::string& tool_id) {
  if (!contents_.get() || tool_id.empty())
    return;
  if (!contents_->ActivateSceneTool(tool_id)) {
    LOG(WARNING) << "map tool rejected: " << tool_id;
    if (status_)
      status_->SetMessage("tool rejected: " + tool_id);
    return;
  }
  if (status_)
    status_->SetMessage("tool: " + tool_id);
}

void WorkbenchContents::OnTrace(const content::TraceLine& line) {
  if (!contents_.get() || !bottom_)
    return;
  std::vector<std::string> lines;
  contents_->trace_log()->Snapshot(line.channel, &lines);
  int channel = 0;
  if (line.channel == content::kTracePerf)
    channel = 1;
  else if (line.channel == content::kTraceAgent)
    channel = 2;
  bottom_->ShowChannel(channel, lines);
}

void WorkbenchContents::on_gesture_applied(
    const content::GestureApplied& event) {
  if (!status_)
    return;
  char buf[64];
  _snprintf_s(buf, sizeof(buf), _TRUNCATE, "XY %d, %d", event.x, event.y);
  status_->SetCoordText(buf);
}

void WorkbenchContents::on_extent_changed(
    const content::ExtentChanged& event) {
  if (!status_)
    return;
  const double width_deg = std::fabs(event.east - event.west);
  // Rough web-mercator-ish scale at mid latitude; display only.
  const double mid_lat = 0.5 * (event.north + event.south);
  const double meters =
      width_deg * 111320.0 * std::cos(mid_lat * 3.141592653589793 / 180.0);
  const int px = map_page_ ? std::max(1, map_page_->width()) : 1;
  const double scale = meters / static_cast<double>(px) * 96.0 / 0.0254;
  char buf[64];
  if (scale > 10.0)
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "1:%.0f", scale);
  else
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "1:-");
  status_->SetScaleText(buf);
}

void WorkbenchContents::StartSession() {
  content::EnsureProductPresentersRegistered();
  contents_.reset(new content::GisContents());
  contents_->trace_log()->AddObserver(this);
  contents_->event_bus()->add_observer(this);
  views::Widget* widget = GetWidget();
  HWND parent = widget ? views::HWNDForWidget(widget) : NULL;
  int width = map_page_ ? map_page_->width() : 0;
  int height = map_page_ ? map_page_->height() : 0;
  if (width < 2 && view_tabs_)
    width = view_tabs_->width();
  if (height < 2 && view_tabs_)
    height = view_tabs_->height();
  if (width < 2)
    width = 640;
  if (height < 2)
    height = 480;
  StartWorkbenchSession(contents_.get(), this, map_page_, view0_page_, parent,
                        width, height);
  LoadWorkbenchPlugins(contents_.get(), &plugin_registry_, &plugin_loader_,
                       tools_panel_);
  debug_agent_.Bind(contents_->tool_session(), contents_->trace_log(), NULL,
                    parent);
  debug_agent_.set_contents(contents_.get());
  if (!debug_agent_.Start())
    LOG(WARNING) << "debug-agent MCP did not start; ExecLine still works";
  ApplySceneMode();
  UpdateStatusMessage();
}

void ShowWorkbench() {
  WorkbenchContents* contents = new WorkbenchContents();
  // DIP design size; CreateWindowWithBounds path is physical for Win32.
  const gfx::Rect kDipBounds(60, 60, 1100, 720);
  const gfx::Rect kPhysicalBounds = ui::win::DIPToScreenRect(kDipBounds);
  views::Widget* widget =
      views::Widget::CreateWindowWithBounds(contents, kPhysicalBounds);
  if (!widget) {
    LOG(ERROR) << "failed to create mgis workbench";
    delete contents;
    return;
  }
  // Re-assert after HWND exists (per-monitor DPI) and after session/DX12,
  // which can starve the UI thread long enough that a pre-Show GetDpiForWindow
  // still reports 96.
  ForceWorkbenchBounds(widget, kDipBounds);
  widget->Show();
  contents->EnsureWindowMenu();
  contents->StartSession();
  if (CommandLine::ForCurrentProcess()->HasSwitch("scene3d"))
    contents->SelectViewTab(kView0);
  ForceWorkbenchBounds(widget, kDipBounds);
  if (MessageLoopForUI::current()) {
    MessageLoopForUI::current()->PostTask(
        FROM_HERE,
        base::Bind(&ForceWorkbenchBounds, widget, kDipBounds));
    MessageLoopForUI::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&ForceWorkbenchBounds, widget, kDipBounds),
        base::TimeDelta::FromMilliseconds(500));
    MessageLoopForUI::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&ForceWorkbenchBounds, widget, kDipBounds),
        base::TimeDelta::FromMilliseconds(2000));
  }
  LOG(INFO) << "mgis workbench shown";
}

WorkbenchContents* CreateWorkbenchContents() {
  return new WorkbenchContents();
}

void SetQuitWhenWorkbenchCloses(bool quit) {
  g_quit_when_main_closes = quit;
}

}  // namespace mgis
