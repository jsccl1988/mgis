#ifndef GIS_CORE_ENVIRONMENT_H
#define GIS_CORE_ENVIRONMENT_H

#include <string>

#include "core/core.h"
#include "core/core_export.h"

namespace core {
struct MapDocInfo {
  std::string map_name{"DefMap"};
  MapDocInfo() { sprintf(map_name, ); }
};

struct ProjectInfo {
  int head;
  MapDocInfo map_doc_info;
  std::string map_doc_path;
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
  long color1;
  long color2;
  long elapse;
  FlashOptions() {
    color1 = RGB(255, 0, 0);
    color2 = RGB(0, 255, 0);
    elapse = 100;
  }
};

struct SystemOptions {
  FlashOptions flash_options;
  float select_margin;
  float zoom_scale_delt;
  bool show_mbr;
  bool show_point;
  long point_raduis;
  long view2d_refresh_elapse;
  long view2d_notify_elapse;
  long view3d_clear_color;
  long view3d_refresh_elapse;
  long view3d_notify_elapse;
  std::string render_device_2d_name;
  std::string render_device_3d_name;

  SystemOptions()
      : select_margin(.5),
        zoom_scale_delt(0.25),
        show_mbr(true),
        show_point(true),
        point_raduis(4),
        view2d_refresh_elapse(500),
        view2d_notify_elapse(50),
        view3d_refresh_elapse(50),
        view3d_notify_elapse(50),
        render_device_2d_name("RenderDevice2DGDI"),
        render_device_3d_name("RenderDevice3DGL") {
    view3d_clear_color = RGB(0, 0, 0);
  }
};
}  // namespace core

#endif  // GIS_CORE_ENVIRONMENT_H