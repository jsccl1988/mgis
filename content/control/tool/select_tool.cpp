// Copyright (c) 2024 The MGIS Authors.
// All rights reserved.
#include "content/control/tool/select_tool.h"

#include "content/control/tool/tool_factory.h"

namespace content {
const static base::NameString kSelectToolName = L"选取";
SelectTool::SelectTool() : captured_(false), result_layer_(NULL) {
  SetName(kSelectToolName.c_str());
}

SelectTool::~SelectTool() {
  // SAFE_DELETE(m_gQDes.pQueryGeom);

  this->EndDelegate();

  // DataSourceMgr::DestoryMemVecLayer(result_layer_);

  UnRegisterMessage();
}

int SelectTool::Init(HWND hwnd, H2DRENDERDEVICE render_device,
                     DelegateCommit delegate_commit, void *to_follow) {
  if (ERR_NONE != Tool::Init(render_device, hwnd, delegate_commit, to_follow)) {
    return ERR_FAILURE;
  }

  // result_layer_ = DataSourceMgr::CreateMemVecLayer();
  // result_layer_->Open("");

  auto& environment = content::Environment::GetInstance();
  auto &system_options = environment.get()->GetSystemOptions();

  select_margin_ = system_options.select_margin;

  AppendFunctionItems(L"点选", TOOL_MESSAGE_SELECT_POINTSEL,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"框选", TOOL_MESSAGE_SELECT_RECTSEL,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"多边形选取", TOOL_MESSAGE_SELECT_POLYGONSEL,
                      FIG_2DVIEW | FIG_2DMFMENU);
  AppendFunctionItems(L"清除选择要素", TOOL_MESSAGE_SELECT_CLEAR,
                      FIG_2DVIEW | FIG_2DMFMENU);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SELECT_POINTSEL);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SELECT_RECTSEL);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SELECT_POLYGONSEL);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SELECT_CLEAR);

  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_SET_SEL_MODE);
  TOOL_APPEND_MESSAGE(TOOL_MESSAGE_GET_SEL_MODE);

  RegisterMessage();

  return ERR_NONE;
}

int SelectTool::AuxDraw() { return Tool::AuxDraw(); }

int SelectTool::Timer() { return Tool::AuxDraw(); }

int SelectTool::Notify(MessageListener::Message &message) {
  if (message.source_window != hwnd_) return ERR_NONE;

  switch (message.id) {
    case TOOL_MESSAGE_DEFAULT_PROCESS: {
    } break;
    case TOOL_MESSAGE_SELECT_POINTSEL: {
      select_mode_ = ST_Point;

      OnSetSelMode();
      message.modify = true;

      SetActive();
    } break;
    case TOOL_MESSAGE_SELECT_RECTSEL: {
      select_mode_ = ST_Rect;

      OnSetSelMode();
      message.modify = true;

      SetActive();
    } break;
    case TOOL_MESSAGE_SELECT_POLYGONSEL: {
      select_mode_ = ST_Polygon;

      OnSetSelMode();
      message.modify = true;

      SetActive();
    } break;
    case TOOL_MESSAGE_SELECT_CLEAR: {
      // result_layer_->DeleteAll();

      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_FLASH_DATA;
      message.source_window = hwnd_;
      message.wparam = WPARAM(result_layer_);
      message.lparam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(TOOL_BROADCAST, message);

      SetActive();
    } break;

    case TOOL_MESSAGE_SET_SEL_MODE: {
      select_mode_ = eSelectMode(*(uint16_t *)message.wparam);

      OnSetSelMode();

      message.modify = true;
    } break;
    case TOOL_MESSAGE_GET_SEL_MODE: {
      *(uint16_t *)message.wparam = select_mode_;
    } break;
    case TOOL_MESSAGE_RET_DELEGATE: {
      uint16_t ret_type = *(uint16_t *)message.lparam;
      // m_gQDes.pQueryGeom = ((OGRGeometry *)message.wparam)->Clone();

      OnRetDelegate(ret_type);
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

void SelectTool::OnRetDelegate(int nRetType) {
  switch (nRetType) {
    case TOOL_MESSAGE_RET_INPUT_POINT: {
      if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
        // result_layer_->DeleteAll();
      }

      // m_gQDes.select_margin = select_margin_ / render_device_->GetBlc();
      // m_pOperMap->QueryFeature(&m_gQDes, &m_pQDes, result_layer_,
      //                          m_nLayerFeaType);

      // SAFE_DELETE(m_gQDes.pQueryGeom);

      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_FLASH_DATA;
      message.source_window = hwnd_;
      message.wparam = WPARAM(result_layer_);
      message.lparam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(TOOL_BROADCAST, message);

      message.id = TOOL_MESSAGE_START_FLASH;
      message.wparam = message.lparam = NULL;
      PostIAToolMessage(TOOL_BROADCAST, message);

      // if (result_layer_->GetFeatureCount() == 1) {
      //   Show2DFeatureInfoDlg(result_layer_->GetFeature(0));
      // }
    } break;
    case TOOL_MESSAGE_RET_INPUT_LINE: {
      if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
        // result_layer_->DeleteAll();
      }

      // m_gQDes.select_margin = select_margin_ / render_device_->GetBlc();
      // m_pOperMap->QueryFeature(&m_gQDes, &m_pQDes, result_layer_,
      //                          m_nLayerFeaType);

      // SAFE_DELETE(m_gQDes.pQueryGeom);

      MessageListener::Message message;
      message.id = TOOL_MESSAGE_SET_FLASH_DATA;
      message.source_window = hwnd_;
      message.wparam = WPARAM(result_layer_);
      message.lparam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(TOOL_BROADCAST, message);

      message.id = TOOL_MESSAGE_START_FLASH;
      message.wparam = message.lparam = NULL;
      PostIAToolMessage(TOOL_BROADCAST, message);

      uint32_t unID = C_INVALID_UINT_VALUE;

      // if (result_layer_->GetFeatureCount() > 1) {
      //   vector<uint32_t> vIDs;

      //   result_layer_->MoveFirst();
      //   while (!result_layer_->IsEnd()) {
      //     Feature *pFea = result_layer_->GetFeature();
      //     vIDs.push_back((uint32_t)pFea->GetID());

      //     result_layer_->MoveNext();
      //   }

      //   SelectOneDlg(unID, vIDs);
      //   Show2DFeatureInfoDlg(result_layer_->GetFeatureByID(unID));
      // } else if (result_layer_->GetFeatureCount() == 1) {
      //   result_layer_->MoveFirst();
      //   Show2DFeatureInfoDlg(result_layer_->GetFeature());
      // }
    } break;
    case TOOL_MESSAGE_RET_INPUT_SURFACE: {
    } break;
  }
}

void SelectTool::OnSetSelMode(void) {
  auto& environment = content::Environment::GetInstance();

  this->EndDelegate();

  Tool *input_tool = NULL;

  switch (select_mode_) {
    case ST_Point: {
      ToolFactory::CreateTool(input_tool, ToolType::InputPoint);

      if (NULL != input_tool) {
        input_tool->SetToolStyleName(styleSonfig.curve_style_);

        if (ERR_NONE == input_tool->Init(render_device_, m_pOperMap, hwnd_)) {
          uint16_t upoint_sizeType = PT_Dot;

          MessageListener::Message message;
          message.id = TOOL_MESSAGE_SET_INPUT_LINE_TYPE;
          message.source_window = hwnd_;
          message.wparam = WPARAM(&upoint_sizeType);
          message.lparam = NULL;
          input_tool->Notify(message);

          this->BeginDelegate(input_tool);
        }
      }
    } break;
    case ST_Polygon: {
      ToolFactory::CreateTool(input_tool, ToolType::InputLine);

      if (NULL != input_tool) {
        input_tool->SetToolStyleName(styleSonfig.curve_style_);

        if (ERR_NONE == input_tool->Init(render_device_, m_pOperMap, hwnd_)) {
          uint16_t unLineType = LT_LinearRing;

          MessageListener::Message message;
          message.id = TOOL_MESSAGE_SET_INPUT_LINE_TYPE;
          message.source_window = hwnd_;
          message.wparam = WPARAM(&unLineType);
          message.lparam = NULL;

          input_tool->Notify(message);

          this->BeginDelegate(input_tool);
        }
      }
    } break;
    case ST_Rect: {
      ToolFactory::CreateTool(input_tool, ToolType::InputLine);

      if (NULL != input_tool) {
        input_tool->SetToolStyleName(styleSonfig.curve_style_);

        if (ERR_NONE == input_tool->Init(render_device_, m_pOperMap, hwnd_)) {
          uint16_t unLineType = LT_Rect;

          MessageListener::Message message;
          message.id = TOOL_MESSAGE_SET_INPUT_LINE_TYPE;
          message.source_window = hwnd_;
          message.wparam = WPARAM(&unLineType);
          message.lparam = NULL;

          input_tool->Notify(message);

          this->BeginDelegate(input_tool);
        }
      }
    } break;
  }
}

int SelectTool::KeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t flags) {
  if (delegate_target_ != NULL && !delegate_target_->IsOperatorDone())
    return delegate_target_->KeyDown(nChar, nRepCnt, flags);
  else {
    if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
      switch (nChar) {
        case 'c':
        case 'C': {
          // result_layer_->DeleteAll();

          MessageListener::Message message;
          message.id = TOOL_MESSAGE_SET_FLASH_DATA;
          message.source_window = hwnd_;
          message.wparam = WPARAM(result_layer_);
          message.lparam = LPARAM(&m_nLayerFeaType);
          PostIAToolMessage(TOOL_BROADCAST, message);

          SetActive();
        } break;
      }
    }
  }

  return Tool::KeyDown(nChar, nRepCnt, flags);
}
}  // namespace content
