// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#include "content/control/tool/input_line_tool.h"

#include "content/common/environment.h"
#include "gfx/2d/renderer/style.h"

static const base::NameString kInputLineTool = L"输入线";

namespace content {
InputLineTool::InputLineTool()
    : geometry_(NULL), is_drag_(false), is_delay_(false), append_type_(-1) {
  SetName(kInputLineTool.c_str());
}

InputLineTool::~InputLineTool() {
  SAFE_DELETE(geometry_);

  UnRegisterMsg();
}

int InputLineTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                        DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(render_device, hwnd, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto &style_manager = gfx2d::StyleManager::GetInstance();
  auto *style = style_manager.get()->GetStyle(style_name_.c_str());
  if (style) {
    style->SetStyleType(ST_PenDesc);
  }

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_INPUT_LINE_TYPE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_INPUT_LINE_TYPE);

  RegisterMessage();

  return ERR_NONE;
}

int InputLineTool::AuxDraw() { return ERR_NONE; }

int InputLineTool::Notify(MessageListener::Message &message) {
  if (message.source_window != hwnd_) {
    return ERR_NONE;
  }

  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
    } break;
    case TOOL_MESSAGE_SET_INPUT_LINE_TYPE: {
      append_type_ = *(uint16_t *)message.wparam;
    } break;
    case TOOL_MESSAGE_GET_INPUT_LINE_TYPE: {
      *(uint16_t *)message.wparam = append_type_;
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputLineTool::LButtonDown(uint32_t flags, Point point) {
  switch (append_type_) {
    case LT_Rect:
      AppendRect(MS_LButtonDown, point);
      break;
    case LT_LineString:
      AppendLineString(MS_LButtonDown, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(MS_LButtonDown, point);
      break;
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputLineTool::MouseMove(uint32_t flags, Point point) {
  switch (append_type_) {
    case LT_Rect:
      AppendRect(MS_MouseMove, point);
      break;
    case LT_LineString:
      AppendLineString(MS_MouseMove, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(MS_MouseMove, point);
      break;
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputLineTool::LButtonUp(uint32_t flags, Point point) {
  switch (append_type_) {
    case LT_Rect:
      AppendRect(MS_LButtonUp, point);
      break;
    case LT_LineString:
      AppendLineString(MS_LButtonUp, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(MS_LButtonUp, point);
      break;
    default:
      break;
  }

  return ERR_NONE;
}

int InputLineTool::RButtonDown(uint32_t flags, Point point) {
  switch (append_type_) {
    case LT_Rect:
      AppendRect(MS_RButtonDown, point);
      break;
    case LT_LineString:
      AppendLineString(MS_RButtonDown, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(MS_RButtonDown, point);
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

int InputLineTool::MouseWheel(uint32_t flags, int16_t z_delta, Point point) {
  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void InputLineTool::AppendLineString(uint32_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;

      if (!is_drag_) {
        float x{0.f}, y{0.f};
        OGRLineString *line_string = new OGRLineString();

        render_device_->DPToLP(point.x, point.y, x, y);
        line_string->addPoint(x, y);

        geometry_ = line_string;
        is_drag_ = true;

        SetOperatorDone(false);
        SetEnableContexMenu(false);
      } else {
        OGRLineString *line_string = (OGRLineString *)geometry_;
        render_device_->DPToLP(point.x, point.y, x, y);
        line_string->addPoint(x, y);
      }
    }

    break;
    case MS_RButtonDown:
      if (is_drag_) {
        is_drag_ = 0;
        current_point_ = point;

        OGRLineString *line_string = (OGRLineString *)geometry_;
        EndAppendLine();

        is_delay_ = true;
      }
      break;
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

void InputLineTool::AppendRect(uint32_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;
      is_drag_ = true;
      step_ = 0;

      float x{0.f}, y{0.f};
      OGRLinearRing *linear_ring = new OGRLinearRing();
      render_device_->DPToLP(point.x, point.y, x, y);
      linear_ring->addPoint(x, y);
      geometry_ = linear_ring;

      SetOperatorDone(false);
      SetEnableContexMenu(false);
    } break;
    case MS_LButtonUp: {
      if (is_drag_) {
        is_drag_ = false;
        current_point_ = point;

        EndAppendLine();

        if (ERR_NONE == render_device_->BeginRender(
                            gfx2d::RenderDevice::RB_DIRECT, true, false))
          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
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
          float x1{0.f}, y1{0.f}, x2{0.f}, y2{0.f};
          OGRLinearRing linear_ring1, linear_ring2;
          render_device_->DPToLP(origin_point_.x, origin_point_.y, x1, y1);
          render_device_->DPToLP(prev_point_.x, prev_point_.y, x2, y2);
          linear_ring1.setPoint(0, x1, y1);
          linear_ring1.setPoint(1, x2, y1);
          linear_ring1.setPoint(2, x2, y2);
          linear_ring1.setPoint(3, x1, y2);
          linear_ring1.closeRings();

          render_device_->DPToLP(current_point_.x, current_point_.y, x2, y2);
          linear_ring2.setPoint(0, x1, y1);
          linear_ring2.setPoint(1, x2, y1);
          linear_ring2.setPoint(2, x2, y2);
          linear_ring2.setPoint(3, x1, y2);
          linear_ring2.closeRings();

          render_device_->DrawLinearRing(&linear_ring1);
          render_device_->DrawLinearRing(&linear_ring2);

          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
        }

        float x1{0.f}, y1{0.f}, x2{0.f}, y2{0.f};
        OGRLinearRing *linear_ring = (OGRLinearRing *)geometry_;
        render_device_->DPToLP(origin_point_.x, origin_point_.y, x1, y1);
        render_device_->DPToLP(current_point_.x, current_point_.y, x2, y2);
        linear_ring->setPoint(1, x2, y1);
        linear_ring->setPoint(2, x2, y2);
        linear_ring->setPoint(3, x1, y2);
        linear_ring->closeRings();
      }
    } break;
    default:
      break;
  }
}

void InputLineTool::AppendLinearRing(uint32_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;

      float x{0.f}, y{0.f};
      if (!is_drag_) {
        OGRLinearRing *linear_ring = new OGRLinearRing();
        render_device_->DPToLP(point.x, point.y, x, y);
        linear_ring->addPoint(x, y);
        geometry_ = linear_ring;
        is_drag_ = true;
        step_ = 0;
        SetOperatorDone(false);
        SetEnableContexMenu(false);
      } else {
        OGRLinearRing *linear_ring = (OGRLinearRing *)geometry_;
        render_device_->DPToLP(point.x, point.y, x, y);
        linear_ring->addPoint(x, y);
      }

      step_++;
    }

    break;
    case MS_RButtonDown: {
      if (is_drag_) {
        OGRLinearRing *linear_ring = (OGRLinearRing *)geometry_;
        current_point_ = point;
        if (step_ < 3) {
          ::MessageBeep(0xFFFFFFFF);
        } else {
          is_drag_ = 0;
          is_delay_ = true;
          step_ = 0;

          auto &style_manager = gfx2d::StyleManager::GetInstance();
          auto *style = style_manager.get()->GetStyle(style_name_.c_str());
          if (ERR_NONE ==
              render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, false,
                                          style, R2_NOTXORPEN)) {
            float x{0.f}, y{0.f};
            OGRLineString line_string;
            render_device_->DPToLP(current_point_.x, current_point_.y, x, y);
            line_string.setPoint(0, x, y);
            render_device_->DPToLP(current_point_.x, current_point_.y, x, y);
            line_string.setPoint(1, linear_ring->getX(0), linear_ring->getY(0));

            render_device_->DrawLine(fpts, 2, true);
            render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);

            render_device_->Render();
          }

          linear_ring->CloseRings();
          EndAppendLine();

          if (ERR_NONE ==
              render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, true))
            render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
        }
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
          OGRLineString line_string;
          float x{0.f}, y{0.f};
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

void InputLineTool::EndAppendLine(void) {
  render_device_->Refresh();

  uint16_t ret_type = TOOL_MESSAGE_RET_INPUT_LINE;
  MessageListener::Message message message;

  message.id = TOOL_MESSAGE_RET_DELEGATE;
  message.source_window = hwnd_;
  message.wparam = WPARAM(geometry_);
  message.lparam = LPARAM(&ret_type);

  CommitDelegate(message);

  SAFE_DELETE(geometry_);

  SetOperatorDone(true);
}
}  // namespace content