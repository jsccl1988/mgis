// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_SESSION_SESSION_H_
#define APP_MGIS_SESSION_SESSION_H_

#include "content/public/browser/gis_contents.h"
#include "content/public/browser/trace_log.h"
#include "plugin/loader.h"
#include "plugin/registry.h"

namespace mgis {

class ToolPluginPanel;
class ViewportSurface;

// Attaches the scene child under |parent_hwnd|, then parents the active-mode
// HWND into |map_page| or |view0_page|. Does not register ViewHost hooks.
bool StartWorkbenchSession(content::GisContents* contents,
                           content::TraceLog::Observer* trace_observer,
                           ViewportSurface* map_page,
                           ViewportSurface* view0_page,
                           void* parent_hwnd,
                           int width,
                           int height);

// Detach/Attach NativeViewHost so the active SceneMode child is visible.
void ReparentSceneSurfaces(content::GisContents* contents,
                           ViewportSurface* map_page,
                           ViewportSurface* view0_page);

// Scans UI/plugins then plugin/sample, loads DLLs into |contents|->plugin_host,
// and refreshes |panel|. |registry| and |loader| are owned by the workbench.
bool LoadWorkbenchPlugins(content::GisContents* contents,
                          plugin::Registry* registry,
                          plugin::Loader* loader,
                          ToolPluginPanel* panel);

}  // namespace mgis

#endif  // APP_MGIS_SESSION_SESSION_H_
