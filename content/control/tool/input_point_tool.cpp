// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/input_point_tool.h"

#include "content/common/environment.h"
#include "gfx/2d/renderer/style.h"

namespace content {
const static base::NameString kInputPointTool = L"输入点";
InputPointTool::InputPointTool()
    : geometry_(NULL),
      is_drag_(false),
      is_delay_(false),
      append_type_(-1),
      angle_(0.) {
  SetName(kInputPointTool.c_str());
}

InputPointTool::~InputPointTool() {
  SAFE_DELETE(geometry_);

  UnRegisterMsg();
}

int InputPointTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                         DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(render_device, hwnd, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto &style_manager = gfx2d::StyleManager::GetInstance();
  auto *style = style_manager.get()->GetStyle(style_name_.c_str());
  style->SetStyleType(ST_PenDesc | ST_BrushDesc | ST_SymbolDesc | ST_AnnoDesc);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_INPUT_POINT_TYPE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_INPUT_POINT_TYPE);

  RegisterMessage();

  return ERR_NONE;
}

int InputPointTool::AuxDraw() { return ERR_NONE; }

int InputPointTool::Notify(MessageListener::Message &message) {
  if (message.source_window != hwnd_) {
    return ERR_NONE;
  }

  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
    } break;
    case TOOL_MESSAGE_SET_INPUT_POINT_TYPE: {
      append_type_ = *(uint16_t *)message.wparam;
    } break;
    case TOOL_MESSAGE_GET_INPUT_ANNO_ANGLE: {
      *(float *)message.wparam = angle_;
    } break;
    case TOOL_MESSAGE_GET_INPUT_POINT_TYPE: {
      *(uint16_t *)message.wparam = append_type_;
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::LButtonDown(uint32_t flags, Point point) {
  SetOperatorDone(false);

  switch (append_type_) {
    case PT_Image:
      AppendImage(MS_LButtonDown, point);
      break;
    case PT_Text:
      AppendText(MS_LButtonDown, point);
      break;
    case PT_Dot:
      AppendDot(MS_LButtonDown, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::MouseMove(uint32_t flags, Point point) {
  switch (append_type_) {
    case PT_Image:
      AppendImage(MS_MouseMove, point);
      break;
    case PT_Text:
      AppendText(MS_MouseMove, point);
      break;
    case PT_Dot:
      AppendDot(MS_MouseMove, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::LButtonUp(uint32_t flags, Point point) {
  switch (append_type_) {
    case PT_Image:
      AppendImage(MS_LButtonUp, point);
      break;
    case PT_Text:
      AppendText(MS_LButtonUp, point);
      break;
    case PT_Dot:
      AppendDot(MS_LButtonUp, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::RButtonDown(uint32_t flags, Point point) {
  switch (append_type_) {
    case PT_Image:
      AppendImage(MS_RButtonDown, point);
      break;
    case PT_Text:
      AppendText(MS_RButtonDown, point);
      break;
    case PT_Dot:
      AppendDot(MS_RButtonDown, point);
      break;
    default:
      break;
  }

  if (IsOperatorDone()) {
    if (is_delay_) {
      is_delay_ = false;

    } else {
      SetEnableContexMenu(true);
    }
  }
  return ERR_NONE;
}

int InputPointTool::MouseWheel(uint32_t flags, int16_t z_delta, Point point) {
  return ERR_NONE;
}

void InputPointTool::AppendImage(int16_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;

      float x, y;
      render_device_->DPToLP(origin_point_.x, origin_point_.y, x, y);
      geometry_ = new Point(x, y);

      SetOperatorDone(false);
      SetEnableContexMenu(false);
      EndAppendPoint();
    } break;
  }
}

void InputPointTool::AppendText(int16_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;
      is_drag_ = 1;

      SetOperatorDone(false);
      SetEnableContexMenu(false);
    }

    break;
    case MS_LButtonUp: {
      if (is_drag_) {
        is_drag_ = 0;
        current_point_ = point;

        auto &style_manager = gfx2d::StyleManager::GetInstance();
        auto *style = style_manager.get()->GetStyle(style_name_.c_str());
        if (ERR_NONE ==
            render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, false,
                                        style, R2_NOTXORPEN)) {
          float x{0.f}, y{0.f};
          OGRLineString line_string;
          render_device_->DPToLP(origin_point_.x, origin_point_.y, x, y);
          line_string.setPoint(0, x, y);
          render_device_->DPToLP(current_point_.x, current_point_.y, x, y);
          line_string.setPoint(1, x, y);

          render_device_->DrawLineString(&line_string);
          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
        }

        float x{0.f}, y{0.f};
        angle_ = 0;
        render_device_->DPToLP(origin_point_.x, origin_point_.y, x, y);
        geometry_ = new Point(x, y);
        if (origin_point_ != current_point_) {
          angle_ = atanf(float(current_point_.y - origin_point_.y) /
                         float(current_point_.x - origin_point_.x));
        }

        EndAppendPoint();
      }
    } break;
    case MS_MouseMove: {
      prev_point_ = current_point_;
      current_point_ = point;
      if (is_drag_) {
        auto &style_manager = gfx2d::StyleManager::GetInstance();
        auto *style = style_manager.get()->GetStyle(style_name_.c_str());
        if (ERR_NONE ==
            render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, false,
                                        style, R2_NOTXORPEN)) {
          float x{0.f}, y{0.f};
          OGRLineString line_string;
          render_device_->DPToLP(origin_point_.x, origin_point_.y, x, y);
          line_string.setPoint(0, x, y);
          render_device_->DPToLP(prev_point_.x, prev_point_.y, x, y);
          line_string.setPoint(1, x, y);
          render_device_->DrawLineString(&line_string);

          render_device_->DPToLP(current_point_.x, current_point_.y, x, y);
          line_string.setPoint(1, x, y);
          render_device_->DrawLineString(&line_string);

          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
        }
      }
    } break;
    default:
      break;
  }
}

void InputPointTool::AppendDot(int16_t mouse_status, _Core::Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;

      float x{0.f}, y{0.f};
      render_device_->DPToLP(origin_point_.x, origin_point_.y, x, y);
      geometry_ = new OGRPoint(x, y);
      SetOperatorDone(false);
      SetEnableContexMenu(false);
      EndAppendPoint();
    } break;
  }
}

void InputPointTool::EndAppendPoint() {
  render_device_->Refresh();

  uint16_t ret_type = TOOL_MESSAGE_RET_INPUT_POINT;
  MessageListener::Message message message;

  message.id = TOOL_MESSAGE_RET_DELEGATE;
  message.source_window = hwnd_;
  message.wparam = WPARAM(geometry_);
  message.lParam = LPARAM(&ret_type);

  CommitDelegate(TOOL_MESSAGE_RET_DELEGATE, message);

  SAFE_DELETE(geometry_);

  SetOperatorDone(true);
}
}  // namespace content