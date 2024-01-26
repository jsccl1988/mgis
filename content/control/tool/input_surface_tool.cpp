// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/input_surface_tool.h"

#include "content/common/environment.h"
#include "gfx/2d/renderer/style.h"

namespace content {
const static base::NameString kIuputSurfaceTool = L"输入面";
InputSurfaceTool::InputSurfaceTool()
    : geometry_(NULL),
      is_drag_(false),
      is_delay_(false),
      append_type_(-1),
      step_(0) {
  SetName(kIuputSurfaceTool.c_str());
}

InputSurfaceTool::~InputSurfaceTool() {
  SAFE_DELETE(geometry_);

  UnRegisterMsg();
}

int InputSurfaceTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                           DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(render_device, hwnd, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto *style_manager = gfx2d::StyleManager::GetSingletonPtr();
  auto *style = style_manager->GetStyle(style_name_.c_str());
  style->SetStyleType(ST_PenDesc | ST_BrushDesc);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_INPUT_SURFACE_TYPE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_INPUT_SURFACE_TYPE);

  RegisterMessage();

  return ERR_NONE;
}

int InputSurfaceTool::AuxDraw() { return ERR_NONE; }

int InputSurfaceTool::Notify(MessageListener::Message &message) {
  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
      ;
    } break;
    case TOOL_MESSAGE_SET_INPUT_SURFACE_TYPE: {
      append_type_ = *(uint16_t *)message.wparam;
    } break;
    case TOOL_MESSAGE_GET_INPUT_SURFACE_TYPE: {
      *(uint16_t *)message.wparam = append_type_;
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputSurfaceTool::LButtonDown(uint32_t flags, Point point) {
  SetOperatorDone(false);

  switch (append_type_) {
    case ST_Rect:
      AppendRect(MS_LButtonDown, point);
      break;
    case ST_Polygon:
      AppendPolygon(MS_LButtonDown, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputSurfaceTool::MouseMove(uint32_t flags, Point point) {
  switch (append_type_) {
    case ST_Rect:
      AppendRect(MS_MouseMove, point);
      break;
    case ST_Polygon:
      AppendPolygon(MS_MouseMove, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputSurfaceTool::LButtonUp(uint32_t flags, Point point) {
  switch (append_type_) {
    case ST_Rect:
      AppendRect(MS_LButtonUp, point);
      break;
    case ST_Polygon:
      AppendPolygon(MS_LButtonUp, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputSurfaceTool::RButtonDown(uint32_t flags, Point point) {
  switch (append_type_) {
    case ST_Rect:
      AppendRect(MS_RButtonDown, point);
      break;
    case ST_Polygon:
      AppendPolygon(MS_RButtonDown, point);
      break;
    default:
      break;
  }

  if (IsOperatorDone()) {
    if (is_delay_)
      is_delay_ = false;
    else
      SetEnableContexMenu(true);
  }

  return ERR_NONE;
}

int InputSurfaceTool::MouseWheel(uint32_t flags, int16_t z_delta, Point point) {
  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void InputSurfaceTool::AppendRect(uint32_t mouse_status, Point point) {
  float x1, y1;
  static Point lpts[2];

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
    case MS_LButtonUp:
      if (is_drag_) {
        is_drag_ = false;
        current_point_ = point;

        OGRLinearRing *linear_ring = (OGRLinearRing *)geometry_;
        OGRPolygon *polygon = new OGRPolygon();
        polygon->addRingDirectly(linear_ring);
        geometry_ = polygon;

        EndAppendSurface();

        if (ERR_NONE == render_device_->BeginRender(
                            gfx2d::RenderDevice::RB_DIRECT, true, false))
          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
      }
      break;
    case MS_MouseMove: {
      prev_point_ = current_point_;
      current_point_ = point;

      if (is_drag_) {
        auto *style_manager = gfx2d::StyleManager::GetSingletonPtr();
        auto *style = style_manager->GetStyle(style_name_.c_str());
        if (ERR_NONE ==
            render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, false,
                                        style, R2_NOTXORPEN)) {
          float x1{0.f}, y1{0.f}, x2{0.f}, y2{0.f};
          OGRLinearRing linear_ring1, linear_ring2;
          render_device_->DPToLP(origin_point_.x, origin_point_.y, x1, y1);
          render_device_->DPToLP(prev_point_.x, prev_point_.y, x2, y2);
          linear_ring1->setPoint(0, x1, y1);
          linear_ring1->setPoint(1, x2, y1);
          linear_ring1->setPoint(2, x2, y2);
          linear_ring1->setPoint(3, x1, y2);

          render_device_->DPToLP(current_point_.x, current_point_.y, x2, y2);
          linear_ring2->setPoint(0, x1, y1);
          linear_ring2->setPoint(1, x2, y1);
          linear_ring2->setPoint(2, x2, y2);
          linear_ring2->setPoint(3, x1, y2);

          render_device_->DrawLinearRing(&linear_ring1);
          render_device_->DrawLinearRing(&linear_ring2);

          render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);
        }

        float x1{0.f}, y1{0.f}, x2{0.f}, y2{0.f};
        OGRLinearRing *linear_ring = (OGRLinearRing *)geometry_;
        render_device_->DPToLP(origin_point_.x, origin_point_.y, x1, y1);
        render_device_->DPToLP(current_point_.x, current_point_.y, x2, y2);
        linear_ring->SetPoint(1, x2, y1);
        linear_ring->SetPoint(2, x2, y2);
        linear_ring->SetPoint(3, x1, y2);
        linear_ring->CloseRings();
      }
    } break;
    default:
      break;
  }
}

void InputSurfaceTool::AppendPolygon(uint32_t mouse_status, Point point) {
  static OGRLineString line_string;

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
        if (step_ < 3) {
          MessageBeep(0xFFFFFFFF);
        } else {
          is_drag_ = 0;
          current_point_ = point;
          is_delay_ = true;
          step_ = 0;

          linear_ring->CloseRings();

          Polygon *polygon = new Polygon();
          polygon->AddRingDirectly(linear_ring);
          geometry_ = polygon;
          EndAppendSurface();

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
        auto *style_manager = gfx2d::StyleManager::GetSingletonPtr();
        auto *style = style_manager->GetStyle(style_name_.c_str());
        if (ERR_NONE ==
            render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT, false,
                                        style, R2_NOTXORPEN)) {
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

void InputSurfaceTool::EndAppendSurface() {
  render_device_->Refresh();

  uint16_t ret_type = TOOL_MESSAGE_RET_INPUT_SURFACE;

  MessageListener::Message message message;
  message.id = TOOL_MESSAGE_RET_DELEGATE;
  message.source_window = hwnd_;
  message.wparam = WPARAM(geometry_);
  message.lParam = LPARAM(&ret_type);

  CommitDelegate(message);

  SAFE_DELETE(geometry_);
  SetOperatorDone(true);
}
}  // namespace content