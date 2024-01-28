// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/edit_tool.h"

#include "content/common/environment.h"
#include "content/control/tool/tool_factory.h"

namespace content {
const static base::NameString kEditToolName = L"编辑";
EditTool::EditTool() { SetName(kEditToolName.c_str()); }

EditTool::~EditTool() {
  this->EndDelegate();
  UnRegisterMessage();
}

int EditTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                   DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(hwnd, render_device, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  auto &style_manager = gfx2d::StyleManager::GetInstance();
  auto *style = style_manager.get()->GetStyle(style_name_.c_str());
  if (style) {
    style->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                        gfx2d::ST_SymbolDesc | gfx2d::ST_AnnoDesc);
  }

  auto &environment = content::Environment::GetInstance();
  auto &system_options = environment.get()->GetSystemOptions();

  AppendFunctionItems(L"添加点", TOOL_MESSAGE_APPEND_POINT_DOT_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"添加折线", TOOL_MESSAGE_APPEND_LINE_LINESTRING_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"添加矩形线", TOOL_MESSAGE_APPEND_LINE_RECT_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"添加闭合折线",
                      TOOL_MESSAGE_APPEND_LINE_LINEARRING_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"添加矩形区", TOOL_MESSAGE_APPEND_SURFACE_RECT_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"添加多边形区",
                      TOOL_MESSAGE_APPEND_SURFACE_POLYGON_FEATURE,
                      FIG_2DVIEW | FIG_2DMFMENU);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_POINT_DOT_FEATURE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_LINE_LINESTRING_FEATURE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_LINE_RECT_FEATURE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_LINE_LINEARRING_FEATURE);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_SURFACE_RECT_FEATURE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_APPEND_SURFACE_POLYGON_FEATURE);

  RegisterMessage();

  return ERR_NONE;
}

int EditTool::AuxDraw() { return Tool::AuxDraw(); }

int EditTool::Timer() { return Tool::AuxDraw(); }

int EditTool::Notify(MessageListener::Message &message) {
  if (message.source_window != hwnd_) {
    return ERR_NONE;
  }

  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
    } break;
    case TOOL_MESSAGE_APPEND_POINT_DOT_FEATURE: {
      uint16_t type = PT_Dot;
      OnInputPointFeature(type);
      message.modify = true;
    } break;
    //////////////////////////////////////////////////////////////////////////
    case TOOL_MESSAGE_APPEND_LINE_RECT_FEATURE: {
      uint16_t type = LT_Rect;
      OnInputLineFeature(type);
      message.modify = true;
    } break;
    case TOOL_MESSAGE_APPEND_LINE_LINEARRING_FEATURE: {
      uint16_t type = LT_LinearRing;
      OnInputLineFeature(type);
      message.modify = true;
    } break;
    case TOOL_MESSAGE_APPEND_LINE_LINESTRING_FEATURE: {
      uint16_t type = LT_LineString;
      OnInputLineFeature(type);
      message.modify = true;
    } break;
    case TOOL_MESSAGE_APPEND_SURFACE_RECT_FEATURE: {
      uint16_t type = ST_Rect;
      OnInputSurfaceFeature(type);
      message.modify = true;
    } break;
    case TOOL_MESSAGE_APPEND_SURFACE_POLYGON_FEATURE: {
      uint16_t type = ST_Polygon;
      OnInputSurfaceFeature(type);
      message.modify = true;
    } break;
    //////////////////////////////////////////////////////////////////////////
    case TOOL_MESSAGE_RET_DELEGATE: {
      uint16_t ret_type = *(uint16_t *)message.lparam;
      geometry_ = ((OGRGeometry *)message.wparam)->clone();
      OnRetDelegate(ret_type);
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

void EditTool::OnRetDelegate(int nRetType) {
  switch (nRetType) {
    case TOOL_MESSAGE_RET_INPUT_POINT: {
      uint16_t type = 0;
      MessageListener::Message message;
      message.id = TOOL_MESSAGE_GET_INPUT_POINT_TYPE;
      message.source_window = hwnd_;
      message.wparam = WPARAM(&type);
      delegate_target_->Notify(message);
      AppendPointFeature(type);
    } break;
    case TOOL_MESSAGE_RET_INPUT_LINE: {
      AppendLineFeature();
    } break;
    case TOOL_MESSAGE_RET_INPUT_SURFACE: {
      AppendSurfaceFeature();
    } break;
  }
}

//////////////////////////////////////////////////////////////////////////
void EditTool::OnInputPointFeature(uint16_t type) {
  auto &environment = content::Environment::GetInstance();
  auto &style_options = environment.get()->GetSystemOptions().style_options;

  this->EndDelegate();

  Tool *input_tool = nullptr;
  ToolFactory::CreateTool(input_tool, content::ToolFactory::InputPoint);

  if (nullptr != input_tool) {
    input_tool->SetToolStyleName(style_options.point_style.c_str());

    if (ERR_NONE == input_tool->Init(hwnd_, render_device_)) {
      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_INPUT_POINT_TYPE;
      message.source_window = hwnd_;
      message.wparam = WPARAM(&type);
      input_tool->Notify(message);
      this->BeginDelegate(input_tool);
    }
  }

  SetActive();
}

void EditTool::OnInputLineFeature(uint16_t type) {
  auto &environment = content::Environment::GetInstance();
  auto &style_options = environment.get()->GetSystemOptions().style_options;

  this->EndDelegate();

  Tool *input_tool = nullptr;
  ToolFactory::CreateTool(input_tool, content::ToolFactory::InputLine);

  if (nullptr != input_tool) {
    input_tool->SetToolStyleName(style_options.curve_style.c_str());

    if (ERR_NONE == input_tool->Init(hwnd_, render_device_)) {
      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_INPUT_LINE_TYPE;
      message.source_window = hwnd_;
      message.wparam = WPARAM(&type);
      input_tool->Notify(message);

      this->BeginDelegate(input_tool);
    }
  }

  SetActive();
}

void EditTool::OnInputSurfaceFeature(uint16_t type) {
  auto &environment = content::Environment::GetInstance();
  auto &style_options = environment.get()->GetSystemOptions().style_options;

  this->EndDelegate();

  Tool *input_tool = nullptr;
  ToolFactory::CreateTool(input_tool, content::ToolFactory::InputSurface);

  if (nullptr != input_tool) {
    input_tool->SetToolStyleName(style_options.surface_style.c_str());

    if (ERR_NONE == input_tool->Init(hwnd_, render_device_)) {
      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_INPUT_SURFACE_TYPE;
      message.source_window = hwnd_;
      message.wparam = WPARAM(&type);
      input_tool->Notify(message);

      this->BeginDelegate(input_tool);
    }
  }

  SetActive();
}
//////////////////////////////////////////////////////////////////////////
void EditTool::AppendPointFeature(uint16_t type) {
  switch (type) {
    case PT_Dot: {
      AppendDotFeature();
    } break;
  }
}

void EditTool::AppendDotFeature() { geometry_ = nullptr; }

void EditTool::AppendLineFeature(void) { geometry_ = nullptr; }

void EditTool::AppendSurfaceFeature() { geometry_ = nullptr; }

}  // namespace content
