// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/flash_tool.h"

#include "content/common/environment.h"
#include "gfx/2d/renderer/style.h"

namespace content {
const static base::NameString kFlashTool = L"闪烁";
FlashTool::FlashTool()
    : is_style1_(true), result_layer_(NULL), scale_delta_(0.15) {
  SetName(kFlashTool.c_str());
}

FlashTool::~FlashTool() {
  // DataSourceMgr::DestoryMemVecLayer(result_layer_);

  UnRegisterMessage();
}

int FlashTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                    DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(hwnd, render_device, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto& environment = content::Environment::GetInstance();
  auto system_options = environment.get()->GetSystemOptions();

  scale_delta_ = system_options.zoom_scale_delt;

  // result_layer_ = DataSourceMgr::CreateMemVecLayer();
  // result_layer_->Open("");

  AppendFunctionItems(L"开始闪烁¸", TOOL_MESSAGE_START_FLASH, FIG_2DVIEW);
  AppendFunctionItems(L"停止闪烁¸", TOOL_MESSAGE_STOP_FLASH, FIG_2DVIEW);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_START_FLASH);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_STOP_FLASH);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_FLASH_MODE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_FLASH_MODE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_FLASH_PARAMETERS);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_FLASH_DATA);

  RegisterMessage();

  return ERR_NONE;
}

int FlashTool::AuxDraw() {
  if (is_flash_) {
    auto& environment = content::Environment::GetInstance();
    auto system_options = environment.get()->GetSystemOptions();
    auto &style_manager = gfx2d::StyleManager::GetInstance();
    auto *style = style_manager.get()->GetStyle(flash_style_.c_str());
    if (ERR_NONE == render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT,
                                                true, style, R2_COPYPEN)) {
      render_device_->RenderLayer(result_layer_, R2_COPYPEN);
      render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
    }
  }

  return ERR_NONE;
}

int FlashTool::Timer() {
  is_style1_ = !is_style1_;
  if (is_style1_) {
    flash_style_ = flash_style1_;
  } else {
    flash_style_ = flash_style2_;
  }

  return ERR_NONE;
}

int FlashTool::Notify(MessageListener::Message &message) {
  if (message.source_window != hwnd_) return ERR_NONE;

  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
    } break;
    case TOOL_MESSAGE_STOP_FLASH: {
      is_flash_ = false;

      if (ERR_NONE ==
          render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, true,
                                      NULL, R2_COPYPEN)) {
        render_device_->RenderLayer((OGRLayer *)NULL, R2_COPYPEN);
        render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
      }

      render_device_->Refresh();
    } break;
    case TOOL_MESSAGE_START_FLASH: {
      is_flash_ = true;
    } break;
    case TOOL_MESSAGE_SET_FLASH_MODE: {
      flash_mode_ = eFlashMode(*(uint16_t *)message.wparam);
    } break;
    case TOOL_MESSAGE_GET_FLASH_MODE: {
      *(uint16_t *)message.wparam = flash_mode_;
    } break;
    case TOOL_MESSAGE_GET_STATUS: {
      *(uint16_t *)message.wparam = (is_flash_) ? 1 : 0;
    } break;
    case TOOL_MESSAGE_SET_FLASH_PARAMETERS: {
      //
    } break;
    case TOOL_MESSAGE_SET_FLASH_DATA: {
      // int nLayerFeaType = *(int *)message.lparam;
      // OGRLayer *pSrcLayer = (OGRLayer *)message.wparam;
      // result_layer_->DeleteAll();
      // CopyLayer(result_layer_, pSrcLayer);

      auto& environment = content::Environment::GetInstance();
      auto system_options = environment.get()->GetSystemOptions();

      // switch (nLayerFeaType) {
      //   case FeatureType::FtChildImage:
      //   case FeatureType::FtDot:
      //   case FeatureType::FtAnno: {
      //     flash_style1_ = system_options.style_options.dot_flash_style1;
      //     flash_style2_ = system_options.style_options.dot_flash_style2;
      //   } break;
      //   case FeatureType::FtCurve: {
      //     flash_style1_ = system_options.style_options.line_flash_style1;
      //     flash_style2_ = system_options.style_options.line_flash_style2;
      //   } break;
      //   case FeatureType::FtSurface: {
      //     flash_style1_ =
      //     system_options.style_options.region_flash_style1; flash_style2_
      //     = system_options.style_options.region_flash_style2;
      //   } break;
      // }
    } break;
    default:
      break;
  }
  return ERR_NONE;
}
}  // namespace content
