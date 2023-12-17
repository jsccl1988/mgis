#ifndef FRAMEWORK_EXPORT_ENVIRONMENT_H
#define FRAMEWORK_EXPORT_ENVIRONMENT_H

#include <string>

#include "framework/framework.h"
#include "framework/core_export.h"

namespace framework {
struct MapDocInfo {
  std::string name{"DefMap"};
  std::string path{"DefMap"};
};

struct ProjectInfo {
  int head{0};
  MapDocInfo map_doc_info;
};

struct StyleConfig {
  std::string point_style{"DefPointStyle"};
  std::string curve_style{"DefLineStyle"};
  std::string region_style{"DefRegionStyle"};
  std::string aux_style{"DefAuxStyle"};

  std::string dot_flash_style1{"DefAnnoFlashStyle1"};
  std::string dot_flash_style2{"DefAnnoFlashStyle2"};
  std::string line_flash_style1{"DefLineFlashStyle1"};
  std::string line_flash_style2{"DefLineFlashStyle2"};
  std::string region_flash_style1{"DefRegionFlashStyle1"};
  std::string region_flash_style2{"DefRegionFlashStyle2"};
};

struct FlashOptions {
  long color1{RGB(255, 0, 0)};
  long color2{RGB(0, 255, 0)};
  long elapse{100};
};

struct SystemOptions {
  FlashOptions flash_options;
  float select_margin{0.5};
  float zoom_scale_delt{0.25};
  bool show_mbr{false};
  bool show_point{false};
  long point_raduis{4};
  long view2d_refresh_elapse{500};
  long view2d_notify_elapse{50};
  long view3d_clear_color{RGB(0, 0, 0)};
  long view3d_refresh_elapse{50};
  long view3d_notify_elapse{50};
  std::string render_device_2d_name{"RenderDevice2DGDI"};
  std::string render_device_3d_name{"RenderDevice3DGL"};
};
}  // namespace framework

#endif  // FRAMEWORK_EXPORT_ENVIRONMENT_H