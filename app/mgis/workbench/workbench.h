// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_WORKBENCH_WORKBENCH_H_
#define APP_MGIS_WORKBENCH_WORKBENCH_H_

#include <string>
#include <string_view>

#include "app/mgis/horizon/catalog_tabs.h"
#include "app/mgis/horizon/map_toolbar.h"
#include "app/mgis/view_id.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/string16.h"
#include "content/browser/agent/debug/debug_agent.h"
#include "content/public/browser/event_bus.h"
#include "content/public/browser/gis_contents.h"
#include "content/public/browser/trace_log.h"
#include "plugin/loader.h"
#include "plugin/registry.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "ui/views/markup/loader/markup_loader.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class TabbedPane;
class Widget;
class Menu;
class View;
}

namespace content {
class GisContents;
}

namespace mgis {

class BottomPanel;
class StatusBar;
class ToolPluginPanel;
class ViewportSurface;

// Product workbench aligned with SmartGIS chrome: toolbar, Layers/Sources/Maps,
// Map|3D, inspector groups, bottom five tabs, status bar.
class WorkbenchContents : public views::WidgetDelegateView,
                          public views::TabbedPaneListener,
                          public CatalogTabs::Observer,
                          public MapToolbar::Delegate,
                          public content::TraceLog::Observer,
                          public content::EventBus::Observer {
 public:
  WorkbenchContents();
  virtual ~WorkbenchContents();

  void SelectViewTab(ViewId id);
  ViewId ActiveViewId() const { return active_; }

  bool HasLayerTree() const;
  bool HasEntityTree() const;

  void EnsureWindowMenu();
  void StartSession();

  // WidgetDelegateView:
  virtual bool CanResize() const OVERRIDE;
  virtual bool CanMaximize() const OVERRIDE;
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual views::View* GetContentsView() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual void WindowClosing() OVERRIDE;

  // TabbedPaneListener (center view tabs):
  virtual void TabSelectedAt(int index) OVERRIDE;

  // CatalogTabs::Observer:
  virtual void OnCatalogTabSelected(int index) OVERRIDE;

  // MapToolbar::Delegate:
  virtual void OnMapToolSelected(const std::string& tool_id) OVERRIDE;

  // TraceLog::Observer:
  virtual void OnTrace(const content::TraceLine& line) OVERRIDE;

  // EventBus::Observer:
  virtual void on_gesture_applied(
      const content::GestureApplied& event) OVERRIDE;
  virtual void on_extent_changed(
      const content::ExtentChanged& event) OVERRIDE;

 private:
  class WindowMenuDelegate;

  void ApplyViewTab(ViewId id);
  void ApplySceneMode();
  void UpdateStatusMessage();
  bool InflateChromeFromMarkup();
  void MountSlot(std::string_view slot_id, views::View* child);
  void LayoutSceneSurfaces();

  ViewId active_;
  bool syncing_;
  views::MarkupRoot markup_;
  views::View* chrome_root_;
  MapToolbar* toolbar_;
  CatalogTabs* catalog_;
  views::TabbedPane* view_tabs_;
  ViewportSurface* map_page_;
  ViewportSurface* view0_page_;
  ToolPluginPanel* tools_panel_;
  BottomPanel* bottom_;
  StatusBar* status_;
  scoped_ptr<WindowMenuDelegate> menu_delegate_;
  scoped_ptr<views::Menu> menu_;
  // Declared before |contents_| so GisContents (PluginHost) dies before DLLs.
  plugin::Registry plugin_registry_;
  plugin::Loader plugin_loader_;
  scoped_ptr<content::GisContents> contents_;
  // After |contents_| so ~DebugAgent runs while GisContents is still alive.
  content::DebugAgent debug_agent_;

  DISALLOW_COPY_AND_ASSIGN(WorkbenchContents);
};

// Creates the main product window (workbench only; no satellite viewports).
void ShowWorkbench();

// Allocates a workbench contents view for harness hosts. Caller transfers
// ownership to Widget::CreateWindowWithBounds.
WorkbenchContents* CreateWorkbenchContents();

// When false, closing the workbench does not Quit the UI message loop.
// Render-check closes widgets without Run().
void SetQuitWhenWorkbenchCloses(bool quit);

}  // namespace mgis

#endif  // APP_MGIS_WORKBENCH_WORKBENCH_H_
