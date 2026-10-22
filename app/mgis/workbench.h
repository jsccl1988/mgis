// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_WORKBENCH_H_
#define APP_MGIS_WORKBENCH_H_

#include "app/mgis/catalog_tabs.h"
#include "app/mgis/view_id.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/string16.h"
#include "ui/views/controls/slider.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "content/public/browser/gis_contents.h"
#include "content/public/browser/trace_log.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class TabbedPane;
class Widget;
class Menu;
}

namespace content {
class GisContents;
}

namespace mgis {

class BottomPanel;
class ToolPluginPanel;
class ViewportPlaceholder;
class ViewportSurface;

// Product workbench: left catalog tabs, center 2D/3D, right tools, bottom
// console/trace/agent. Bidirectional SelectViewTab linkage.
class WorkbenchContents : public views::WidgetDelegateView,
                          public views::SliderListener,
                          public views::TabbedPaneListener,
                          public CatalogTabs::Observer,
                          public content::TraceLog::Observer {
 public:
  WorkbenchContents();
  virtual ~WorkbenchContents();

  void SelectViewTab(ViewId id);
  ViewId ActiveViewId() const { return active_; }

  bool HasLayerTree() const;
  bool HasEntityTree() const;

  void SetSatellites(views::Widget* map,
                     views::Widget* view0,
                     views::Widget* view1);
  void CloseSatellites();
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

  // SliderListener:
  virtual void SliderValueChanged(views::Slider* sender,
                                  float value,
                                  float old_value,
                                  views::SliderChangeReason reason) OVERRIDE;

  // TabbedPaneListener (center view tabs):
  virtual void TabSelectedAt(int index) OVERRIDE;

  // CatalogTabs::Observer:
  virtual void OnCatalogTabSelected(int index) OVERRIDE;

  // TraceLog::Observer:
  virtual void OnTrace(const content::TraceLine& line) OVERRIDE;

 private:
  class WindowMenuDelegate;

  void ApplyViewTab(ViewId id);

  ViewId active_;
  bool syncing_;
  views::Slider* slider_;
  CatalogTabs* catalog_;
  views::TabbedPane* view_tabs_;
  ViewportPlaceholder* map_page_;
  ViewportSurface* view0_page_;
  ToolPluginPanel* tools_panel_;
  BottomPanel* bottom_;
  scoped_ptr<WindowMenuDelegate> menu_delegate_;
  scoped_ptr<views::Menu> menu_;
  scoped_ptr<content::GisContents> contents_;
  views::Widget* map_;
  views::Widget* view0_;
  views::Widget* view1_;

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

#endif  // APP_MGIS_WORKBENCH_H_
