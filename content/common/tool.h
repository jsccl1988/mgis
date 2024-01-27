// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_TOOL_H
#define CONTENT_COMMON_TOOL_H

#include <stack>
#include <vector>

#include "base/memory/singleton.h"
#include "content/common/message.h"
#include "content/content.h"
#include "content/content_export.h"
#include "content/public/map_types.h"
#include "gfx/2d/renderer/render_device.h"

#define TOOL_MESSAGE_KEY(message) (MESSAGE_KEY(message, hwnd_))

#define TOOL_APPEND_MESSAGE(message) \
  { AppendMessage(TOOL_MESSAGE_KEY(message)); }

namespace content {
using DelegateCommit = int (*)(MessageListener::Message &message);

class Tool : public MessageListener {
 public:
  Tool();
  virtual ~Tool(void);

 public:
  virtual int Init(HWND hwnd, H2DRENDERDEVICE render_device,
                   DelegateCommit delegate_commit = nullptr,
                   void *to_follow = nullptr);
  virtual int AuxDraw(void);
  virtual int Timer(void);

  virtual int SetActive();

 public:
  inline HWND GetOwnerWnd() { return hwnd_; }
  inline H2DRENDERDEVICE GetRenderDevice(void) { return render_device_; }

  inline void SetToolStyleName(const char *name) { style_name_ = name; }
  inline const char *GetToolStyleName(void) { return style_name_.c_str(); }

  inline bool IsOperatorDone(void) {
    if (delegate_target_ != nullptr) {
      return delegate_target_->IsOperatorDone();
    }

    return done_;
  }
  inline void SetOperatorDone(bool done) {
    if (delegate_target_ != nullptr) {
      delegate_target_->SetOperatorDone(done);
    }

    done_ = done;
  }

  inline bool IsEnableContexMenu(void) {
    if (delegate_target_ != nullptr) {
      return delegate_target_->IsEnableContexMenu();
    }

    return enable_context_menu_;
  }
  inline void SetEnableContexMenu(bool enable) {
    if (delegate_target_ != nullptr) {
      delegate_target_->SetEnableContexMenu(enable);
    }

    enable_context_menu_ = enable;
  }

  virtual int RegisterMessage(void);
  virtual int UnRegisterMessage(void);

  virtual int SetCursor(void);

 public:
  virtual bool SetDelegateSource(Tool *delegate_source_tool);
  virtual bool BeginDelegate(Tool *delegate_tool);
  virtual int EndDelegate(bool will_release_target_tool = true);
  virtual int CommitDelegate(MessageListener::Message &message);

 public:
  virtual int LButtonDown(uint32_t nFlags, Point point);
  virtual int LButtonUp(uint32_t nFlags, Point point);
  virtual int LButtonDClick(uint32_t nFlags, Point point);

  virtual int RButtonDown(uint32_t nFlags, Point point);
  virtual int RButtonUp(uint32_t nFlags, Point point);
  virtual int RButtonDClick(uint32_t nFlags, Point point);

  virtual int MouseMove(uint32_t nFlags, Point point);
  virtual int MouseWheel(uint32_t nFlags, int16_t z_delta, Point point);

  virtual int KeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags);
  virtual int KeyUp(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags);

 protected:
  int oper_;
  bool done_;
  bool enable_context_menu_;
  HWND hwnd_;
  HCURSOR cross_cursor_;

  H2DRENDERDEVICE render_device_;
  std::string style_name_;

  double scale_delta_;

 protected:
  Tool *delegate_source_;
  Tool *delegate_target_;
  DelegateCommit delegate_commit_;
  void *to_follow_;
};

using ToolPtrs = std::vector<Tool *>;

class ToolManager : public base::Singleton<ToolManager> {
 public:
  long Notify(Tool *tool, MessageListener::Message &message);

  inline void SetActiveTool(Tool *tool) { current_tool_ = tool; }
  inline Tool *GetActiveTool(void) { return current_tool_; }
  inline const Tool *GetActiveTool(void) const { return current_tool_; }

  long RegisterTool(Tool *tool);
  long RemoveTool(Tool *tool);
  long RemoveAllTool(void);

  Tool *GetTool(int index);
  const Tool *GetTool(int index) const;
  inline size_t GetToolCount(void) const { return toolptrs_.size(); }

  long RegisterToolMessage(Tool *tool);
  long UnRegisterToolMessage(Tool *tool);

 protected:
  ToolPtrs toolptrs_;
  Tool *current_tool_;
  Message2Ptr message_2_tools_;
};

long PostToolMessage(Tool *tool, MessageListener::Message &message);
}  // namespace content

#define TOOL_BROADCAST ((content::Tool *)0xFFFF)
#define TOOL_INVALID ((content::Tool *)0x0000)

#define TOOL_POST_MESSAGE(tool, message)                       \
  {                                                            \
    auto &tool_manager = content::ToolManager::GetSingleton(); \
    tool_manager.Notify(tool, message);                        \
  }

#endif  // CONTENT_COMMON_TOOL_H