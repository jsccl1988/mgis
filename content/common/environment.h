// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_ENVIRONMENT_H
#define CONTENT_COMMON_ENVIRONMENT_H

#include <string>

#include "content/content.h"
#include "content/content_export.h"

namespace content {
struct MapDocInfo {
  std::string name{"DefMap"};
  std::string path{"DefMap"};
};

struct ProjectInfo {
  int head{0};
  MapDocInfo map_doc_info;
};

struct StyleOptions {
  std::string point_style{"DefPointStyle"};
  std::string curve_style{"DefLineStyle"};
  std::string region_style{"DefSurfaceStyle"};
  std::string aux_style{"DefAuxStyle"};

  std::string dot_flash_style1{"DefAnnoFlashStyle1"};
  std::string dot_flash_style2{"DefAnnoFlashStyle2"};
  std::string line_flash_style1{"DefLineFlashStyle1"};
  std::string line_flash_style2{"DefLineFlashStyle2"};
  std::string region_flash_style1{"DefSurfaceFlashStyle1"};
  std::string region_flash_style2{"DefSurfaceFlashStyle2"};
};

struct FlashOptions {
  long color1{RGB(255, 0, 0)};
  long color2{RGB(0, 255, 0)};
  long elapse{100};
};

struct SystemOptions {
  StyleOptions style_options;
  FlashOptions flash_options;
  float select_margin{0.5};
  float zoom_scale_delt{0.25};
  bool show_mbr{false};
  bool show_point{false};
  long point_radius{4};
  long view2d_refresh_elapse{500};
  long view2d_notify_elapse{50};
  long view3d_clear_color{RGB(0, 0, 0)};
  long view3d_refresh_elapse{50};
  long view3d_notify_elapse{50};
  std::string render_device_2d{"RenderDevice2DGDI"};
  std::string render_device_3d{"RenderDevice3DGL"};
};

class Environment {
 public:
  static Environment *GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  inline MapDocInfo GetMapDocInfo(void) const { return map_doc_info_; }
  inline void SetMapDocInfo(MapDocInfo &mapDocInfo) {
    map_doc_info_ = mapDocInfo;
  }

  inline ProjectInfo GetProjectInfo(void) const { return project_info_; }
  inline void SetProjectInfo(ProjectInfo &prjInfo) { project_info_ = prjInfo; }

  inline SystemOptions GetSystemOptions(void) const { return system_options_; }
  inline void SetSystemOptions(const SystemOptions &system_options) {
    system_options_ = system_options;
  }

 private:
  MapDocInfo map_doc_info_;
  ProjectInfo project_info_;

  SystemOptions system_options_;

 private:
  Environment(void) {}
  ~Environment(void) {}

  static Environment *singleton_;
};
}  // namespace content

#endif  // CONTENT_COMMON_ENVIRONMENT_H