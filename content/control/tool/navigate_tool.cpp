// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#include "content/control/tool/navigate_tool.h"

#include "content/common/environment.h"
#include "content/control/resource/resource.h"
#include "gfx/2d/renderer/style.h"

namespace content {
const static base::NameString kNavigateTool = L"地图浏览";
NavigateTool::NavigateTool()
    : captured_(false), flashed_(0), view_model_(VM_ZoomOff) {
  SetName(kNavigateTool.c_str());
}

NavigateTool::~NavigateTool() { UnRegisterMessage(); }

int NavigateTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                       DelegateCommit delegate_commit, void* to_follow) {
  if (ERR_NONE != Tool::Init(hwnd, render_device, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto &style_manager = gfx2d::StyleManager::GetInstance();
  auto *style = style_manager.get()->GetStyle(style_name_.c_str());
  if (style) {
    style->SetStyleType(gfx2d::ST_PenDesc);
  }

  UINT cursor_ids[] = {IDC_CURSOR_ZOOMIN, IDC_CURSOR_ZOOMOUT,
                       IDC_CURSOR_ZOOMMOVE, IDC_CURSOR_IDENTIFY};

  int nCount = sizeof(cursor_ids) / sizeof(UINT);

  for (int i = 0; i < nCount; i++) {
    cursors_[i] = ::LoadCursor(::GetModuleHandle(nullptr),
                               MAKEINTRESOURCE(cursor_ids[i]));
  }

  AppendFunctionItems(L"放大", TOOL_MESSAGE_VIEW_ZOOMIN,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"缩小", TOOL_MESSAGE_VIEW_ZOOMOUT,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"移动", TOOL_MESSAGE_VIEW_ZOOMMOVE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"复位", TOOL_MESSAGE_VIEW_ZOOMRESTORE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"刷新", TOOL_MESSAGE_VIEW_ZOOMREFRESH,
                      FIG_2DVIEW | FIG_2DMFMENU);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_VIEW_ZOOMIN);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_VIEW_ZOOMOUT);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_VIEW_ZOOMMOVE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_VIEW_ZOOMRESTORE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_VIEW_ZOOMREFRESH);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_VIEW_MODE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_VIEW_MODE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_SCALEDELT);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_SCALEDELT);

  RegisterMessage();

  return ERR_NONE;
}

int NavigateTool::AuxDraw() { return ERR_NONE; }

int NavigateTool::Timer() { return ERR_NONE; }

int NavigateTool::Notify(MessageListener::Message& message) {
  if (message.source_window != hwnd_) {
    switch (message.id) {
      case TOOL_MESSAGE_VIEW_ZOOMRESTORE: {
        view_model_ = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case TOOL_MESSAGE_VIEW_ZOOMREFRESH: {
        view_model_ = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
      case TOOL_MESSAGE_VIEW_ACTIVE: {
        ::SetForegroundWindow(hwnd_);
      } break;
    }
  } else {
    switch (message.id) {
      case TOOL_MESSAGE_VIEW_ZOOMIN: {
        view_model_ = VM_ZoomIn;
      } break;
      case TOOL_MESSAGE_VIEW_ZOOMOUT: {
        view_model_ = VM_ZoomOut;
      } break;
      case TOOL_MESSAGE_VIEW_ZOOMMOVE: {
        view_model_ = VM_ZoomMove;
      } break;
      case TOOL_MESSAGE_VIEW_ZOOMRESTORE: {
        view_model_ = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case TOOL_MESSAGE_VIEW_ZOOMREFRESH: {
        view_model_ = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
      case TOOL_MESSAGE_SET_VIEW_MODE: {
        view_model_ = eViewMode(*(uint16_t*)message.wparam);

        switch (view_model_) {
          case VM_ZoomRestore:
            ZoomRestore();
            break;
          case VM_ZoomRefresh:
            ZoomRefresh();
            break;
          default:
            break;
        }
      } break;
      case TOOL_MESSAGE_GET_VIEW_MODE: {
        *(uint16_t*)message.wparam = view_model_;
      } break;
      case TOOL_MESSAGE_SET_SCALEDELT: {
        scale_delta_ = *(double*)message.wparam;
      } break;
      case TOOL_MESSAGE_GET_SCALEDELT: {
        *(double*)message.wparam = scale_delta_;
      } break;
    }

    SetActive();
  }

  return ERR_NONE;
}

int NavigateTool::SetCursor(void) {
  switch (view_model_) {
    case VM_ZoomOff:
      ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
      break;
    case VM_ZoomIn:
      ::SetCursor(cursors_[CT_LoupePlus]);
      break;
    case VM_ZoomOut:
      ::SetCursor(cursors_[CT_LoupeMinus]);
      break;
    case VM_ZoomMove:
      ::SetCursor(cursors_[CT_Move]);
      break;
    case VM_ZoomRestore:
      ::SetCursor(cross_cursor_);
      break;
    case VM_ZoomRefresh:
      ::SetCursor(cross_cursor_);
      break;
    default:
      // All other zoom modes
      ::SetCursor(cross_cursor_);
      break;
  }

  return ERR_NONE;
}

int NavigateTool::LButtonDown(uint32_t flags, Point point) {
  SetOperatorDone(false);

  switch (view_model_) {
    case VM_ZoomMove:
      ZoomMove(MS_LButtonDown, point);
      break;
    case VM_ZoomIn:
      ZoomIn(MS_LButtonDown, point);
      break;
    case VM_ZoomOut:
      ZoomOut(MS_LButtonDown, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int NavigateTool::MouseMove(uint32_t flags, Point point) {
  switch (view_model_) {
    case VM_ZoomMove:
      ZoomMove(MS_MouseMove, point);
      break;
    case VM_ZoomIn:
      ZoomIn(MS_MouseMove, point);
      break;
    case VM_ZoomOut:
      ZoomOut(MS_MouseMove, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int NavigateTool::LButtonUp(uint32_t flags, Point point) {
  switch (view_model_) {
    case VM_ZoomMove:
      ZoomMove(MS_LButtonUp, point);
      break;
    case VM_ZoomIn:
      ZoomIn(MS_LButtonUp, point);
      break;
    case VM_ZoomOut:
      ZoomOut(MS_LButtonUp, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int NavigateTool::RButtonDown(uint32_t flags, Point point) {
  switch (view_model_) {
    case VM_ZoomMove:
      ZoomMove(MS_RButtonDown, point);
      break;
    case VM_ZoomIn:
      ZoomIn(MS_RButtonDown, point);
      break;
    case VM_ZoomOut:
      ZoomOut(MS_RButtonDown, point);
      break;
    default:
      break;
  }

  SetOperatorDone(true);

  return ERR_NONE;
}

int NavigateTool::MouseWheel(uint32_t flags, int16_t z_delta, Point point) {
  float scale{0.f};
  if (z_delta < 0) {
    scale = 1 + scale_delta_;
  } else {
    scale = 1 - scale_delta_;
  }

  POINT client_point;
  client_point.x = point.x;
  client_point.y = point.y;
  ::ScreenToClient(hwnd_, &client_point);

  gfx2d::LPoint logic_point;
  render_device_->DPToLP(client_point.x, client_point.y, logic_point.x,
                         logic_point.y);
  render_device_->ZoomScale(logic_point, scale);
  render_device_->Refresh();

  return ERR_NONE;
}

void NavigateTool::ZoomMove(int16_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      captured_ = true;
      ::SetCapture(hwnd_);
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;

      MessageListener::Message message;
      message.id = TOOL_MESSAGE_GET_STATUS;
      message.source_window = hwnd_;
      message.wparam = (WPARAM)(&flashed_);
      PostToolMessage(TOOL_BROADCAST, message);

      if (*(uint16_t*)message.wparam) {
        message.id = TOOL_MESSAGE_STOP_FLASH;
        PostToolMessage(TOOL_BROADCAST, message);
      }
    } break;
    case MS_MouseMove: {
      prev_point_ = current_point_;
      current_point_ = point;

      if (captured_) {
        gfx2d::LPoint current_dop;
        current_dop.x = current_point_.x - origin_point_.x;
        current_dop.y = current_point_.y - origin_point_.y;

        render_device_->SetCurrentDOP(current_dop);
        render_device_->Refresh();
      }
    } break;
    case MS_LButtonUp: {
      prev_point_ = current_point_;
      current_point_ = point;
      if (captured_) {
        captured_ = false;
        ::ReleaseCapture();

        gfx2d::LPoint current_dop;
        current_dop.x = 0;
        current_dop.y = 0;

        render_device_->SetCurrentDOP(current_dop);

        float x1{0.f}, y1{0.f}, x2{0.f}, y2{0.f};
        render_device_->DPToLP(origin_point_.x, origin_point_.y, x1, y1);
        render_device_->DPToLP(point.x, point.y, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();

        if (flashed_) {
          MessageListener::Message message;
          message.id = TOOL_MESSAGE_STOP_FLASH;
          message.source_window = hwnd_;
          PostToolMessage(TOOL_BROADCAST, message);
        }
      }
    } break;
    case MS_RButtonDown: {
      current_point_ = point;
      if (captured_) {
        captured_ = false;
        ::ReleaseCapture();
      }

      view_model_ = VM_ZoomOff;
    } break;
    default:
      break;
  }
}

void NavigateTool::ZoomIn(int16_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown: {
      captured_ = true;
      ::SetCapture(hwnd_);
      origin_point_ = point;
      prev_point_ = point;
      current_point_ = point;
    } break;
    case MS_LButtonUp: {
      if (captured_) {
        prev_point_ = current_point_;
        current_point_ = point;
        captured_ = false;
        ::ReleaseCapture();
        if (!gfx2d::equal(origin_point_, point)) {
          gfx2d::LRect logic_rect;
          gfx2d::DRect device_rect;
          device_rect.x = std::min(origin_point_.x, current_point_.x);
          device_rect.y = std::max(origin_point_.y, current_point_.y);
          device_rect.width = std::abs(origin_point_.x - current_point_.x);
          device_rect.height = -std::abs(origin_point_.y - current_point_.y);

          render_device_->DRectToLRect(device_rect, logic_rect);
          render_device_->ZoomToRect(logic_rect);
        } else {
          gfx2d::LPoint logic_point;
          render_device_->DPToLP(point.x, point.y, logic_point.x,
                                 logic_point.y);
          render_device_->ZoomScale(logic_point, 1 - scale_delta_);
        }

        if (ERR_NONE == render_device_->BeginRender(
                            gfx2d::RenderDevice::RB_IMMEDIATELY, true, nullptr))
          render_device_->EndRender(gfx2d::RenderDevice::RB_IMMEDIATELY);

        render_device_->Refresh();
      }
    } break;
    case MS_MouseMove: {
      prev_point_ = current_point_;
      current_point_ = point;

      if (captured_) {
        auto& style_manager = gfx2d::StyleManager::GetInstance();
        auto* style = style_manager.get()->GetStyle(style_name_.c_str());
        if (ERR_NONE ==
            render_device_->BeginRender(gfx2d::RenderDevice::RB_IMMEDIATELY, false,
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

          render_device_->EndRender(gfx2d::RenderDevice::RB_IMMEDIATELY);
        }
      }
    } break;
    case MS_RButtonDown: {
      current_point_ = point;
      if (captured_) {
        captured_ = false;
        ::ReleaseCapture();
      }

      view_model_ = VM_ZoomOff;
    } break;
  }
}

void NavigateTool::ZoomOut(int16_t mouse_status, Point point) {
  switch (mouse_status) {
    case MS_LButtonDown:
      break;
    case MS_LButtonUp: {
      gfx2d::LPoint logic_point;
      render_device_->DPToLP(point.x, point.y, logic_point.x, logic_point.y);
      render_device_->ZoomScale(logic_point, 1 + scale_delta_);
      render_device_->Refresh();
    } break;
    case MS_MouseMove:
      break;
    case MS_RButtonDown: {
      view_model_ = VM_ZoomOff;
    } break;
  }
}

void NavigateTool::ZoomRestore() {
  gfx2d::LRect logic_rect;
  logic_rect.x = logic_rect.y = 0;
  logic_rect.width = 3840;
  logic_rect.height = 2400;

  render_device_->ZoomToRect(logic_rect);
  render_device_->Refresh();
  // if (m_pOperMap != nullptr) {
  //   Envelope envelope;
  //   fRect logic_rect;

  //   Layer *pLayer = m_pOperMap->GetActiveLayer();
  //   if (pLayer) {
  //     pLayer->CalEnvelope();
  //     pLayer->GetEnvelope(envelope);
  //     EnvelopeToRect(logic_rect, envelope);

  //     float fWidthDiv = logic_rect.Width() / 40;
  //     float fHeightDiv = logic_rect.Height() / 40;

  //     logic_rect.rt.x += fWidthDiv;
  //     logic_rect.rt.y += fHeightDiv;
  //     logic_rect.lb.x -= fWidthDiv;
  //     logic_rect.lb.y -= fHeightDiv;
  //   }
  // }
}

void NavigateTool::ZoomRefresh() {
  RECT client_rect;
  ::GetClientRect(hwnd_, &client_rect);

  gfx2d::DRect rect;
  rect.x = client_rect.left;
  rect.y = client_rect.bottom;
  rect.width = client_rect.right - client_rect.left;
  rect.height = client_rect.top - client_rect.bottom;
  render_device_->RefreshDirectly(rect);
}
}  // namespace content
