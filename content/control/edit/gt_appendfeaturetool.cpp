#include "gt_appendfeaturetool.h"

#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "geo_geometry.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "gt_defs.h"
#include "gt_grouptoolfactory.h"
#include "gt_iacommandreceiver.h"
#include "resource.h "
#include "smt_api.h"
#include "smt_gui_api.h"
#include "sys_sysmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;
using namespace core;

const string CST_STR_APPENDFEATURE_TOOL_NAME = "添加要素";

namespace _GroupTool {
AppendFeatureTool::AppendFeatureTool() : m_pGeom(NULL), m_strAnno("Smart GIS") {
  SetName(CST_STR_APPENDFEATURE_TOOL_NAME.c_str());
}

AppendFeatureTool::~AppendFeatureTool() {
  this->EndDelegate();
  m_pGeom = NULL;

  UnRegisterMessage();
}

int AppendFeatureTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
                            HWND hWnd, pfnToolCallBack pfnCallBack,
                            void *to_follow) {
  if (ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
  pStyle->SetStyleType(ST_PenDesc | ST_BrushDesc | ST_SymbolDesc | ST_AnnoDesc);

  //
  AppendFunctionItems("添加子图", GT_MSG_APPEND_POINT_CHILDIMAGE_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加注记", GT_MSG_APPEND_POINT_ANNO_FEATURE, FIM_2DMFMENU);
  AppendFunctionItems("添加点", GT_MSG_APPEND_POINT_DOT_FEATURE, FIM_2DMFMENU);

  //
  AppendFunctionItems("添加折线", GT_MSG_APPEND_LINE_LINESTRING_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加拉格朗日曲线", GT_MSG_APPEND_LINE_SPLINE_LAG_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加贝奇尔曲线", GT_MSG_APPEND_LINE_SPLINE_BZER_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加B样条曲线", GT_MSG_APPEND_LINE_SPLINE_B_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加三次样条曲线", GT_MSG_APPEND_LINE_SPLINE_3_FEATURE,
                  FIM_2DMFMENU);
  AppendFunctionItems("添加矩形线", GT_MSG_APPEND_LINE_RECT_FEATURE, FIM_2DMFMENU);
  AppendFunctionItems("添加弧线", GT_MSG_APPEND_LINE_ARC_FEATURE, FIM_2DMFMENU);
  AppendFunctionItems("添加闭合折线", GT_MSG_APPEND_LINE_LINEARRING_FEATURE,
                  FIM_2DMFMENU);

  //
  AppendFunctionItems("添加扇形区", GT_MSG_APPEND_SURF_FAN_FEATURE, FIM_2DMFMENU);
  AppendFunctionItems("添加矩形区", GT_MSG_APPEND_SURF_RECT_FEATURE, FIM_2DMFMENU);
  AppendFunctionItems("添加多边形区", GT_MSG_APPEND_SURF_POLYGON_FEATURE,
                  FIM_2DMFMENU);

  IATOOL_APPEND_MSG(GT_MSG_APPEND_POINT_CHILDIMAGE_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_POINT_ANNO_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_POINT_DOT_FEATURE);

  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_LINESTRING_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_SPLINE_LAG_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_SPLINE_BZER_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_SPLINE_B_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_SPLINE_3_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_RECT_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_ARC_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_LINE_LINEARRING_FEATURE);

  IATOOL_APPEND_MSG(GT_MSG_APPEND_SURF_FAN_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_SURF_RECT_FEATURE);
  IATOOL_APPEND_MSG(GT_MSG_APPEND_SURF_POLYGON_FEATURE);

  RegisterMessage();

  return ERR_NONE;
}

int AppendFeatureTool::AuxDraw() { return BaseTool::AuxDraw(); }

int AppendFeatureTool::Timer() { return BaseTool::Timer(); }

void AppendFeatureTool::SetOperMap(Map *pOperMap) {
  BaseTool::SetOperMap(pOperMap);

  m_cmdMgr.ClearAllCommands();
}

int AppendFeatureTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.source_window != m_hWnd) return ERR_NONE;

  SysManager *pSysMgr = SysManager::GetSingletonPtr();
  StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
    } break;
    case GT_MSG_APPEND_POINT_CHILDIMAGE_FEATURE: {
      ushort unType = PT_ChildImage;
      OnInputPointFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_POINT_ANNO_FEATURE: {
      ushort unType = PT_Text;
      OnInputPointFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_POINT_DOT_FEATURE: {
      ushort unType = PT_DOT;
      OnInputPointFeature(unType);
      param.modify = true;
    } break;
    //////////////////////////////////////////////////////////////////////////
    case GT_MSG_APPEND_LINE_RECT_FEATURE: {
      ushort unType = LT_Rect;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_ARC_FEATURE: {
      ushort unType = LT_Arc;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_LINEARRING_FEATURE: {
      ushort unType = LT_LinearRing;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_LINESTRING_FEATURE: {
      ushort unType = LT_LineString;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_SPLINE_LAG_FEATURE: {
      ushort unType = LT_Spline_Lag;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_SPLINE_BZER_FEATURE: {
      ushort unType = LT_Spline_Bzer;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_SPLINE_B_FEATURE: {
      ushort unType = LT_Spline_B;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_LINE_SPLINE_3_FEATURE: {
      ushort unType = LT_Spline_3;
      OnInputLineFeature(unType);
      param.modify = true;
    } break;
    //////////////////////////////////////////////////////////////////////////
    case GT_MSG_APPEND_SURF_FAN_FEATURE: {
      ushort unType = RT_Fan;
      OnInputRegionFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_SURF_RECT_FEATURE: {
      ushort unType = RT_Rect;
      OnInputRegionFeature(unType);
      param.modify = true;
    } break;
    case GT_MSG_APPEND_SURF_POLYGON_FEATURE: {
      ushort unType = RT_Polygon;
      OnInputRegionFeature(unType);
      param.modify = true;
    } break;
    //////////////////////////////////////////////////////////////////////////
    case GT_MSG_RET_DELEGATE: {
      ushort uRetType = *(ushort *)param.lparam;
      m_pGeom = ((Geometry *)param.wparam)->Clone();
      OnRetDelegate(uRetType);
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

void AppendFeatureTool::OnRetDelegate(int nRetType) {
  switch (nRetType) {
    case GT_MSG_RET_INPUT_POINT: {
      ushort upoint_sizeType = 0;
      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = WPARAM(&upoint_sizeType);
      m_pDelegateTag->Notify(GT_MSG_GET_INPUT_POINT_TYPE, param);
      AppendPointFeature(upoint_sizeType);
    } break;
    case GT_MSG_RET_INPUT_LINE: {
      AppendLineFeature();
    } break;
    case GT_MSG_RET_INPUT_REGION: {
      AppendRegionFeature();
    } break;
  }
}

//////////////////////////////////////////////////////////////////////////
void AppendFeatureTool::OnInputPointFeature(ushort unType) {
  SysManager *pSysMgr = SysManager::GetSingletonPtr();
  StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

  this->EndDelegate();

  BaseTool *pInputTool = NULL;
  GroupToolFactory::CreateGroupTool(pInputTool, GroupToolType::GTT_InputPoint);

  if (NULL != pInputTool) {
    pInputTool->SetToolStyleName(styleSonfig.point_style_);

    if (ERR_NONE == pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = WPARAM(&unType);
      pInputTool->Notify(GT_MSG_SET_INPUT_POINT_TYPE, param);
      this->BeginDelegate(pInputTool);
    }
  }

  SetActive();
}

void AppendFeatureTool::OnInputLineFeature(ushort unType) {
  SysManager *pSysMgr = SysManager::GetSingletonPtr();
  StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

  this->EndDelegate();

  BaseTool *pInputTool = NULL;
  GroupToolFactory::CreateGroupTool(pInputTool, GroupToolType::GTT_InputLine);

  if (NULL != pInputTool) {
    pInputTool->SetToolStyleName(styleSonfig.curve_style_);

    if (ERR_NONE == pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = WPARAM(&unType);

      pInputTool->Notify(GT_MSG_SET_INPUT_LINE_TYPE, param);

      this->BeginDelegate(pInputTool);
    }
  }

  SetActive();
}

void AppendFeatureTool::OnInputRegionFeature(ushort unType) {
  SysManager *pSysMgr = SysManager::GetSingletonPtr();
  StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

  this->EndDelegate();

  BaseTool *pInputTool = NULL;
  GroupToolFactory::CreateGroupTool(pInputTool, GroupToolType::GTT_InputRegion);

  if (NULL != pInputTool) {
    pInputTool->SetToolStyleName(styleSonfig.region_style_);

    if (ERR_NONE == pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = WPARAM(&unType);

      pInputTool->Notify(GT_MSG_SET_INPUT_REGION_TYPE, param);

      this->BeginDelegate(pInputTool);
    }
  }

  SetActive();
}
//////////////////////////////////////////////////////////////////////////
void AppendFeatureTool::AppendPointFeature(ushort unType) {
  switch (unType) {
    case PT_ChildImage: {
      AppendChildImageFeature();
    } break;
    case PT_Text: {
      float angle = 0;
      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = WPARAM(&angle);
      m_pDelegateTag->Notify(GT_MSG_GET_INPUT_ANNO_ANGLE, param);
      if (ERR_NONE == InputTextDlg(m_strAnno))
        AppendTextFeature(m_strAnno.c_str(), angle);
    } break;
    case PT_DOT: {
      AppendDotFeature();
    } break;
  }
}

void AppendFeatureTool::AppendChildImageFeature() {
  if (m_pOperMap) {
    SysManager *pSysMgr = SysManager::GetSingletonPtr();
    StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

    Feature *pFeature = new Feature;
    pFeature->SetFeatureType(FeatureType::FtChildImage);
    pFeature->SetStyle(styleSonfig.point_style_);
    pFeature->SetGeometryDirectly(m_pGeom);

    if (m_pOperMap->AppendFeature(pFeature, false)) {
      fRect frt;
      Envelope envelope;
      Point *pPoint = (Point *)m_pGeom;
      float fMargin = 5. / m_pRenderDevice->GetBlc();

      m_pGeom->GetEnvelope(&envelope);
      envelope.Merge(pPoint->GetX() - fMargin, pPoint->GetY() - fMargin);
      envelope.Merge(pPoint->GetX() + fMargin, pPoint->GetY() + fMargin);
      EnvelopeToRect(frt, envelope);

      m_pRenderDevice->Refresh(m_pOperMap, frt);

      //
      Command *command = new Command(new IACommandReceiver(
          m_pRenderDevice, m_pOperMap, pFeature, eIA_AppendFeature));
      m_cmdMgr.PushUndoCommand(command);
    } else
      SAFE_DELETE(pFeature);

    m_pGeom = NULL;
  }
}

void AppendFeatureTool::AppendTextFeature(const char *szAnno, float fangle) {
  if (m_pOperMap) {
    SysManager *pSysMgr = SysManager::GetSingletonPtr();
    StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

    Feature *pFeature = new Feature;

    pFeature->SetFeatureType(FeatureType::FtAnno);
    pFeature->SetStyle(styleSonfig.point_style_);
    pFeature->SetGeometryDirectly(m_pGeom);
    pFeature->SetFieldValue(pFeature->GetFieldIndexByName("anno"), szAnno);
    pFeature->SetFieldValue(pFeature->GetFieldIndexByName("color"),
                            int(RGB(0, 0, 0)));
    pFeature->SetFieldValue(pFeature->GetFieldIndexByName("angle"), fangle);

    if (m_pOperMap->AppendFeature(pFeature, false)) {
      fRect frt;
      Envelope envelope;
      Point *pPoint = (Point *)m_pGeom;
      float fMargin = 5. / m_pRenderDevice->GetBlc();

      m_pGeom->GetEnvelope(&envelope);
      envelope.Merge(pPoint->GetX() - fMargin, pPoint->GetY() - fMargin);
      envelope.Merge(pPoint->GetX() + fMargin, pPoint->GetY() + fMargin);
      EnvelopeToRect(frt, envelope);

      m_pRenderDevice->Refresh(m_pOperMap, frt);

      //
      Command *command = new Command(new IACommandReceiver(
          m_pRenderDevice, m_pOperMap, pFeature, eIA_AppendFeature));
      m_cmdMgr.PushUndoCommand(command);
    } else
      SAFE_DELETE(pFeature);

    m_pGeom = NULL;
  }
}

void AppendFeatureTool::AppendDotFeature() {
  if (m_pOperMap) {
    SysManager *pSysMgr = SysManager::GetSingletonPtr();
    StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

    Feature *pFeature = new Feature;

    pFeature->SetFeatureType(FeatureType::FtDot);
    pFeature->SetStyle(styleSonfig.point_style_);
    pFeature->SetGeometryDirectly(m_pGeom);

    if (m_pOperMap->AppendFeature(pFeature, false)) {
      fRect frt;
      Envelope envelope;
      Point *pPoint = (Point *)m_pGeom;
      float fMargin = 5. / m_pRenderDevice->GetBlc();

      m_pGeom->GetEnvelope(&envelope);
      envelope.Merge(pPoint->GetX() - fMargin, pPoint->GetY() - fMargin);
      envelope.Merge(pPoint->GetX() + fMargin, pPoint->GetY() + fMargin);
      EnvelopeToRect(frt, envelope);

      m_pRenderDevice->Refresh(m_pOperMap, frt);

      //
      Command *command = new Command(new IACommandReceiver(
          m_pRenderDevice, m_pOperMap, pFeature, eIA_AppendFeature));
      m_cmdMgr.PushUndoCommand(command);
    } else
      SAFE_DELETE(pFeature);

    m_pGeom = NULL;
  }
}

void AppendFeatureTool::AppendLineFeature(void) {
  if (m_pOperMap) {
    SysManager *pSysMgr = SysManager::GetSingletonPtr();
    StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

    Feature *pFeature = new Feature;
    pFeature->SetFeatureType(FtCurve);
    pFeature->SetStyle(styleSonfig.curve_style_);
    pFeature->SetGeometryDirectly(m_pGeom);
    pFeature->SetFieldValue(pFeature->GetFieldIndexByName("length"),
                            ((Curve *)m_pGeom)->GetLength());

    if (m_pOperMap->AppendFeature(pFeature, false)) {
      fRect frt;
      Envelope envelope;
      float fMargin = 5. / m_pRenderDevice->GetBlc();

      m_pGeom->GetEnvelope(&envelope);
      envelope.Merge(envelope.MinX - fMargin, envelope.MinY - fMargin);
      envelope.Merge(envelope.MaxX + fMargin, envelope.MaxY + fMargin);
      EnvelopeToRect(frt, envelope);

      m_pRenderDevice->Refresh(m_pOperMap, frt);

      //
      Command *command = new Command(new IACommandReceiver(
          m_pRenderDevice, m_pOperMap, pFeature, eIA_AppendFeature));
      m_cmdMgr.PushUndoCommand(command);
    } else
      SAFE_DELETE(pFeature);

    m_pGeom = NULL;
  }
}

void AppendFeatureTool::AppendRegionFeature() {
  if (m_pOperMap) {
    SysManager *pSysMgr = SysManager::GetSingletonPtr();
    StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

    Feature *pFeature = new Feature;
    pFeature->SetFeatureType(FtSurface);
    pFeature->SetStyle(styleSonfig.region_style_);
    pFeature->SetGeometryDirectly(m_pGeom);
    pFeature->SetFieldValue(pFeature->GetFieldIndexByName("area"),
                            ((Surface *)m_pGeom)->GetArea());

    if (m_pOperMap->AppendFeature(pFeature, false)) {
      fRect frt;
      Envelope envelope;
      float fMargin = 5. / m_pRenderDevice->GetBlc();

      m_pGeom->GetEnvelope(&envelope);
      envelope.Merge(envelope.MinX - fMargin, envelope.MinY - fMargin);
      envelope.Merge(envelope.MaxX + fMargin, envelope.MaxY + fMargin);
      EnvelopeToRect(frt, envelope);

      m_pRenderDevice->Refresh(m_pOperMap, frt);

      //
      Command *command = new Command(new IACommandReceiver(
          m_pRenderDevice, m_pOperMap, pFeature, eIA_AppendFeature));
      m_cmdMgr.PushUndoCommand(command);
    } else
      SAFE_DELETE(pFeature);

    m_pGeom = NULL;
  }
}

int AppendFeatureTool::KeyDown(uint nChar, uint nRepCnt, uint nFlags) {
  if (m_pDelegateTag != NULL && !m_pDelegateTag->IsOperDone())
    return m_pDelegateTag->KeyDown(nChar, nRepCnt, nFlags);
  else {
    // if (GetKeyState(VK_CONTROL) & 0x80000000)
    {
      switch (nChar) {
        case 't':
        case 'T': {
          if (m_cmdMgr.CanUndo()) m_cmdMgr.Undo();
        } break;
        case 'y':
        case 'Y': {
          if (m_cmdMgr.CanRedo()) m_cmdMgr.Redo();
        } break;
      }
    }
  }

  return BaseTool::KeyDown(nChar, nRepCnt, nFlags);
}
}  // namespace _GroupTool