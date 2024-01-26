
#include "content/common/tool.h"

#include "content/common/environment.h"

namespace content {
Tool::Tool()
    : hwnd_(NULL),
      oper_(-1),
      done_(false),
      enable_context_menu_(true),
      delegate_source_(NULL),
      delegate_target_(NULL),
      scale_delta_(0.15),
      render_device_(NULL) {
  auto &tool_manager = ToolManager::GetInstance();
  tool_manager.get()->RegisterTool(this);
}

Tool::~Tool() {
  auto &tool_manager = ToolManager::GetInstance();
  tool_manager.get()->RemoveTool(this);
}

int Tool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
               DelegateCommit delegate_commit, void *to_follow) {
  if (NULL == hwnd) {
    return ERR_INVALID_PARAM;
  }

  auto &environment = content::Environment::GetInstance();
  auto system_options = environment.get()->GetSystemOptions();

  delegate_commit_ = delegate_commit;
  to_follow_ = to_follow;
  hwnd_ = hwnd;
  cross_cursor_ = ::LoadCursor(NULL, IDC_CROSS);
  render_device_ = render_device;
  scale_delta_ = system_options.zoom_scale_delt;

  return ERR_NONE;
}

int Tool::RegisterMessage() {
  auto &tool_manager = ToolManager::GetInstance();
  tool_manager.get()->RegisterToolMessage(this);

  return ERR_NONE;
}

int Tool::UnRegisterMessage() {
  auto &tool_manager = ToolManager::GetInstance();
  tool_manager.get()->UnRegisterToolMessage(this);

  return ERR_NONE;
}

int Tool::SetActive() {
  auto &tool_manager = ToolManager::GetInstance();
  tool_manager.get()->SetActiveTool(this);

  return ERR_NONE;
}

int Tool::AuxDraw() {
  if (delegate_target_ != NULL && !delegate_target_->IsOperatorDone()) {
    delegate_target_->AuxDraw();
  }

  return ERR_NONE;
}
int Tool::Timer() {
  if (delegate_target_ != NULL && !delegate_target_->IsOperatorDone()) {
    delegate_target_->Timer();
  }

  return ERR_NONE;
}

bool Tool::SetDelegateSource(Tool *delegate_source_tool) {
  if (delegate_source_) {
    return false;
  }

  delegate_source_ = delegate_source_tool;

  return true;
}

bool Tool::BeginDelegate(Tool *delegate_tool) {
  if (delegate_target_) {
    return false;
  }

  delegate_target_ = delegate_tool;
  delegate_target_->SetDelegateSource(this);

  return true;
}

int Tool::EndDelegate(bool will_release_target_tool) {
  if (will_release_target_tool) {
    SAFE_DELETE(delegate_target_);
  } else {
    delegate_target_ = NULL;
  }

  return ERR_NONE;
}

int Tool::SetCursor(void) {
  if (delegate_target_ != NULL) {
    return delegate_target_->SetCursor();

  } else {
    ::SetCursor(cross_cursor_);
  }

  return ERR_NONE;
}

int Tool::LButtonDown(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->LButtonDown(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::LButtonUp(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
  }
  return delegate_target_->LButtonUp(nFlags, point);

  return ERR_NONE;
}

int Tool::LButtonDClick(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->LButtonDClick(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::RButtonDown(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->RButtonDown(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::RButtonUp(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->RButtonUp(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::RButtonDClick(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->RButtonDClick(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::MouseMove(uint32_t nFlags, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->MouseMove(nFlags, point);
  }

  return ERR_NONE;
}

int Tool::MouseWheel(uint32_t nFlags, int16_t zDelta, Point point) {
  if (delegate_target_ != NULL) {
    return delegate_target_->MouseWheel(nFlags, zDelta, point);
  }

  return ERR_NONE;
}

int Tool::KeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags) {
  if (delegate_target_ != NULL) {
    return delegate_target_->KeyDown(nChar, nRepCnt, nFlags);
  } else {
    switch (nChar) {
      case VK_F5: {
        render_device_->Refresh();
      } break;
      case 'Q': {
        POINT mouse_position;
        float scale{0.f};
        scale = 1 - scale_delta_;
        ::GetCursorPos(&mouse_position);
        ::ScreenToClient(hwnd_, &mouse_position);

        gfx2d::LPoint point(mouse_position.x, mouse_position.y);
        render_device_->ZoomScale(point, scale);
        render_device_->Refresh();
      } break;
      case 'E': {
        POINT mouse_position;
        float scale{0.f};
        scale = 1 + scale_delta_;
        ::GetCursorPos(&mouse_position);
        ::ScreenToClient(hwnd_, &mouse_position);

        gfx2d::LPoint point(mouse_position.x, mouse_position.y);
        render_device_->ZoomScale(point, scale);
        render_device_->Refresh();
      } break;
      case 'R': {
        POINT mouse_position;
        POINT screen_center;
        RECT wndrt;
        ::GetWindowRect(hwnd_, &wndrt);

        screen_center.x = (wndrt.left + wndrt.right) >> 1;
        screen_center.y = (wndrt.top + wndrt.bottom) >> 1;

        ::GetCursorPos(&mouse_position);

        if ((mouse_position.x == screen_center.x) &&
            (mouse_position.y == screen_center.y)) {
          return ERR_NONE;
        }

        ::ScreenToClient(hwnd_, &mouse_position);
        ::ScreenToClient(hwnd_, &screen_center);

        float x1, y1, x2, y2;
        render_device_->DPToLP(mouse_position.x, mouse_position.y, x1, y1);
        render_device_->DPToLP(screen_center.x, screen_center.y, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();
      } break;
      case 'W': {
        float x1, y1, x2, y2;
        render_device_->DPToLP(0, 0, x1, y1);
        render_device_->DPToLP(0, -10, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();
      } break;
      case 'A': {
        float x1, y1, x2, y2;
        render_device_->DPToLP(0, 0, x1, y1);
        render_device_->DPToLP(-10, 0, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();
      } break;
      case 'S': {
        float x1, y1, x2, y2;
        render_device_->DPToLP(0, 0, x1, y1);
        render_device_->DPToLP(0, 10, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();
      } break;
      case 'D': {
        float x1, y1, x2, y2;
        render_device_->DPToLP(0, 0, x1, y1);
        render_device_->DPToLP(10, 0, x2, y2);

        gfx2d::LPoint offset(x2 - x1, y2 - y1);
        render_device_->ZoomMove(offset);
        render_device_->Refresh();
      } break;
      case 'Z': {
        gfx2d::LRect logic_rect;
        logic_rect.x = logic_rect.y = 0;
        logic_rect.width = 3840;
        logic_rect.height = 2400;

        render_device_->ZoomToRect(logic_rect);
        render_device_->Refresh();
      } break;
    }
  }

  return ERR_NONE;
}

int Tool::KeyUp(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags) {
  if (delegate_target_ != NULL) {
    return delegate_target_->KeyUp(nChar, nRepCnt, nFlags);
  }

  return ERR_NONE;
}

int Tool::CommitDelegate(MessageListener::Message &message) {
  message.to_follow = to_follow_;
  if (NULL != delegate_commit_) {
    (*delegate_commit_)(message);
  }

  if (NULL != delegate_source_) {
    delegate_source_->Notify(message);
  }

  return ERR_NONE;
}

long ToolManager::Notify(Tool *tool, MessageListener::Message &message) {
  if (tool == TOOL_INVALID) {
    return ERR_INVALID_PARAM;
  }

  if (tool == TOOL_BROADCAST) {
    Message2Ptr::iterator iter;
    iter = message_2_tools_.find(message.id);
    if (iter != message_2_tools_.end()) {
      Tool *found_tool = (Tool *)iter->second;
      if (found_tool) {
        found_tool->Notify(message);
      }
    } else {
      ToolPtrs dones;
      ToolPtrs ::iterator it = toolptrs_.begin();
      while (it != toolptrs_.end()) {
        if (find(dones.begin(), dones.end(), *it) == dones.end()) {
          message.modify = false;
          dones.push_back(*it);
          (*it++)->Notify(message);
          if (message.modify) {
            it = toolptrs_.begin();
            continue;
          }
        } else
          ++it;
      }
    }
  } else {
    tool->Notify(message);
  }

  return ERR_NONE;
}

long ToolManager::RegisterTool(Tool *tool) {
  ToolPtrs::iterator iter = toolptrs_.begin();

  while (iter != toolptrs_.end()) {
    if ((*iter) == tool) {
      return false;
    }
    ++iter;
  }

  toolptrs_.push_back(tool);

  return ERR_NONE;
}

long ToolManager::RemoveTool(Tool *tool) {
  ToolPtrs::iterator iter = toolptrs_.begin();
  while (iter != toolptrs_.end()) {
    if ((*iter) == tool) {
      toolptrs_.erase(iter);
      if (tool == current_tool_) {
        current_tool_ = NULL;
      }
      break;
    }
    ++iter;
  }

  return true;
}

long ToolManager::RemoveAllTool(void) {
  toolptrs_.clear();
  current_tool_ = NULL;
  return ERR_NONE;
}

Tool *ToolManager::GetTool(int index) {
  if (index < 0 || index >= toolptrs_.size()) return NULL;

  return toolptrs_.at(index);
}

const Tool *ToolManager::GetTool(int index) const {
  if (index < 0 || index >= toolptrs_.size()) return NULL;

  return toolptrs_.at(index);
}

long ToolManager::RegisterToolMessage(Tool *tool) {
  if (tool == NULL) {
    return ERR_INVALID_PARAM;
  }

  Messages messags = tool->GetMessages();
  Message2Ptr::iterator map_iter = message_2_tools_.begin();
  Messages::iterator iter = messags.begin();
  while (iter != messags.end()) {
    if (message_2_tools_.find((*iter)) == message_2_tools_.end()) {
      message_2_tools_.insert(Message2PtrPair((*iter), (void *)tool));
    }
    ++iter;
  }

  return ERR_NONE;
}

long ToolManager::UnRegisterToolMessage(Tool *tool) {
  if (tool == NULL) {
    return ERR_INVALID_PARAM;
  }

  Messages messags = tool->GetMessages();
  Message2Ptr::iterator map_iter = message_2_tools_.begin();
  Messages::iterator iter = messags.begin();

  while (iter != messags.end()) {
    if ((map_iter = message_2_tools_.find((*iter))) != message_2_tools_.end()) {
      message_2_tools_.erase(map_iter);
    }
    ++iter;
  }

  return ERR_NONE;
}

long PostToolMessage(Tool *tool, MessageListener::Message &message) {
  auto &tool_manager = ToolManager::GetInstance();
  return tool_manager.get()->Notify(tool, message);
}
}  // namespace content
