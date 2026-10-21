// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/workbench.h"

#include <algorithm>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "app/mgis/bottom_panel.h"
#include "app/mgis/tool_plugin_panel.h"
#include "app/mgis/viewport_placeholder.h"
#include "app/mgis/viewport_surface.h"
#include "content/public/debug_agent.h"
#include "content/public/gis_contents.h"
#include "content/public/tool_session.h"
#include "content/public/trace_log.h"
#include "content/public/types.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/menu/menu.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/widget/widget.h"
#include "ui/views/win/hwnd_util.h"

namespace mgis {
namespace {

bool g_quit_when_main_closes = true;

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
      slider_(NULL),
      catalog_(NULL),
      view_tabs_(NULL),
      map_page_(NULL),
      view0_page_(NULL),
      tools_panel_(NULL),
      bottom_(NULL),
      map_(NULL),
      view0_(NULL),
      view1_(NULL) {
  menu_delegate_.reset(new WindowMenuDelegate());

  slider_ = new views::Slider(this, views::Slider::HORIZONTAL);
  slider_->SetValue(0.0f);

  catalog_ = new CatalogTabs();
  catalog_->set_observer(this);

  map_page_ = new ViewportPlaceholder(
      ASCIIToUTF16("2D viewport (placeholder)"),
      SkColorSetRGB(0xe8, 0xee, 0xf4));
  view0_page_ = new ViewportSurface();

  view_tabs_ = new views::TabbedPane();
  view_tabs_->set_listener(this);
  view_tabs_->AddTab(ASCIIToUTF16("2D"), map_page_);
  view_tabs_->AddTab(ASCIIToUTF16("3D"), view0_page_);

  tools_panel_ = new ToolPluginPanel();
  bottom_ = new BottomPanel();

  AddChildView(slider_);
  AddChildView(catalog_);
  AddChildView(view_tabs_);
  AddChildView(tools_panel_);
  AddChildView(bottom_);
}

WorkbenchContents::~WorkbenchContents() {
  if (contents_.get())
    contents_->trace_log()->RemoveObserver(this);
}

void WorkbenchContents::SelectViewTab(ViewId id) {
  if (id != kMap && id != kView0)
    id = kMap;
  if (syncing_)
    return;
  if (active_ == id &&
      catalog_ && catalog_->selected_tab_index() == static_cast<int>(id) &&
      view_tabs_ && view_tabs_->selected_tab_index() == static_cast<int>(id)) {
    return;
  }
  ApplyViewTab(id);
}

void WorkbenchContents::ApplyViewTab(ViewId id) {
  syncing_ = true;
  active_ = id;
  const int index = static_cast<int>(id);
  if (catalog_)
    catalog_->SelectTabAt(index);
  if (view_tabs_)
    view_tabs_->SelectTabAt(index);
  syncing_ = false;
  ApplySceneMode();
}

void WorkbenchContents::ApplySceneMode() {
  if (!contents_.get())
    return;
  const content::SceneMode mode = active_ == kView0
                                      ? content::SceneMode::kScene3D
                                      : content::SceneMode::kScene2D;
  contents_->SetSceneMode(mode);
}

bool WorkbenchContents::HasLayerTree() const {
  return catalog_ && catalog_->layer_tree() != NULL;
}

bool WorkbenchContents::HasEntityTree() const {
  return catalog_ && catalog_->entity_tree() != NULL;
}

void WorkbenchContents::SetSatellites(views::Widget* map,
                                      views::Widget* view0,
                                      views::Widget* view1) {
  map_ = map;
  view0_ = view0;
  view1_ = view1;
}

void WorkbenchContents::CloseSatellites() {
  views::Widget* widgets[] = {map_, view0_, view1_};
  map_ = NULL;
  view0_ = NULL;
  view1_ = NULL;
  for (size_t i = 0; i < arraysize(widgets); ++i) {
    if (widgets[i])
      widgets[i]->CloseNow();
  }
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
  menu_->AppendMenuItemWithLabel(1, ASCIIToUTF16("Map"));
  menu_->AppendMenuItemWithLabel(2, ASCIIToUTF16("view0"));
  menu_->AppendMenuItemWithLabel(3, ASCIIToUTF16("view1"));
}

bool WorkbenchContents::CanResize() const { return true; }
bool WorkbenchContents::CanMaximize() const { return true; }

string16 WorkbenchContents::GetWindowTitle() const {
  return ASCIIToUTF16("MView");
}

views::View* WorkbenchContents::GetContentsView() { return this; }

gfx::Size WorkbenchContents::GetPreferredSize() {
  return gfx::Size(1100, 720);
}

void WorkbenchContents::Layout() {
  const int slider_h = 36;
  const int bottom_h = 140;
  const int left_w = 240;
  const int right_w = 200;
  const int mid_h = std::max(0, height() - slider_h - bottom_h);
  const int center_w = std::max(0, width() - left_w - right_w);

  if (slider_)
    slider_->SetBounds(0, 0, width(), slider_h);
  if (catalog_)
    catalog_->SetBounds(0, slider_h, left_w, mid_h);
  if (view_tabs_)
    view_tabs_->SetBounds(left_w, slider_h, center_w, mid_h);
  if (tools_panel_)
    tools_panel_->SetBounds(left_w + center_w, slider_h, right_w, mid_h);
  if (bottom_)
    bottom_->SetBounds(0, slider_h + mid_h, width(), bottom_h);
}

void WorkbenchContents::WindowClosing() {
  CloseSatellites();
  if (g_quit_when_main_closes && MessageLoopForUI::current())
    MessageLoopForUI::current()->Quit();
}

void WorkbenchContents::SliderValueChanged(
    views::Slider* sender,
    float value,
    float old_value,
    views::SliderChangeReason reason) {
  (void)sender;
  (void)value;
  (void)old_value;
  (void)reason;
}

void WorkbenchContents::TabSelectedAt(int index) {
  if (syncing_)
    return;
  SelectViewTab(index == 0 ? kMap : kView0);
}

void WorkbenchContents::OnCatalogTabSelected(int index) {
  if (syncing_)
    return;
  SelectViewTab(index == 0 ? kMap : kView0);
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

void WorkbenchContents::StartSession() {
  contents_.reset(new content::GisContents());
  session_.reset(new content::ToolSession());
  session_->set_event_bus(contents_->event_bus());
  session_->set_trace_log(contents_->trace_log());
  contents_->trace_log()->AddObserver(this);

  views::Widget* widget = GetWidget();
  HWND parent = widget ? views::HWNDForWidget(widget) : NULL;
  int width = view_tabs_ ? view_tabs_->width() : 0;
  int height = view_tabs_ ? view_tabs_->height() : 0;
  if (width < 2)
    width = 640;
  if (height < 2)
    height = 480;
  if (parent) {
    contents_->AttachView(parent, std::string(content::kViewIdScene), width,
                          height);
  }
  ApplySceneMode();

  contents_->trace_log()->Append(content::kTraceConsole,
                                "backend.unavailable maplibre");
  agent_.reset(new content::DebugAgent());
  agent_->Bind(session_.get(), contents_->trace_log(), NULL, parent);
  agent_->ExecLine("help");
  contents_->trace_log()->Append(content::kTraceConsole, "session.ready");
  LOG(INFO) << "mgis content session started";
}

void ShowWorkbench() {
  WorkbenchContents* contents = new WorkbenchContents();
  views::Widget* widget = views::Widget::CreateWindowWithBounds(
      contents, gfx::Rect(60, 60, 1100, 720));
  if (!widget) {
    LOG(ERROR) << "failed to create mgis workbench";
    delete contents;
    return;
  }
  widget->Show();
  contents->EnsureWindowMenu();
  contents->StartSession();
  LOG(INFO) << "mgis workbench shown";
}

WorkbenchContents* CreateWorkbenchContents() {
  return new WorkbenchContents();
}

void SetQuitWhenWorkbenchCloses(bool quit) {
  g_quit_when_main_closes = quit;
}

}  // namespace mgis
