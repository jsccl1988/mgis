#include "gt_3dviewctrltool.h"

#include "bl3d_object.h"
#include "gt_defs.h"
#include "resource.h"


const string CST_STR_3DVIEWCTRL_TOOL_NAME = "三维浏览";

namespace _GroupTool {
3DViewCtrlTool ::3DViewCtrlTool()
    : m_bIsDrag(false),
      m_viewMode(V3DM_Normal),
      m_nWinWidth(0),
      m_nWinHeight(0),
      m_fAngle(0),
      m_pLastTarget(NULL),
      m_pCamera(NULL) {
  SetName(CST_STR_3DVIEWCTRL_TOOL_NAME.c_str());
}

3DViewCtrlTool ::~3DViewCtrlTool() {
  UnRegisterMessage();
  SAFE_DELETE(m_pCamera);
}

int 3DViewCtrlTool ::Init(LP3DRENDERDEVICE p3DRenderDevice, Scene* pScene,
                          HWND hWnd, pfnToolCallBack pfnCallBack,
                          void* to_follow) {
  if (ERR_NONE !=
      Base3DTool::Init(p3DRenderDevice, pScene, hWnd, pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  Vector3 TCenter = Vector3(0, 0, 0);
  m_vOrgEye = TCenter + Vector3(0, 0, 100);
  m_vOrgTarget = TCenter;
  m_vOrgUp = Vector3(0.f, 1.f, 0.f);

  //////////////////////////////////////////////////////////////////////////
  Viewport3D viewport = m_p3DRenderDevice->GetViewport();
  Aabb aabb = m_pScene->GetAabb();

  m_pCamera = new PerspCamera(m_p3DRenderDevice, viewport);
  m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
  m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 500);

  m_pScene->SetSceneCamera(m_pCamera);

  //////////////////////////////////////////////////////////////////////////
  //设置光标
  UINT idCursors[] = {IDC_CURSOR_TRACEBALL, IDC_CURSOR_SPHERECAMERA,
                      IDC_CURSOR_FIRSTPERSON};

  int nCount = sizeof(idCursors) / sizeof(UINT);

  for (int i = 0; i < nCount; i++)
    m_hCursors[i] = ::LoadCursor(g_hInstance, MAKEINTRESOURCE(idCursors[i]));

  //设置选中物体材质
  m_matSel.SetAmbientValue(Color(0, 0, 0));
  m_matSel.SetDiffuseValue(Color(0, 0, 1));
  m_matSel.SetSpecularValue(Color(0, 1, 1));
  m_matSel.SetEmissiveValue(Color(0, 0, 1));
  m_matSel.SetShininessValue(50);

  AppendFuncItems("追踪球", GT_MSG_3DVIEW_TRACEBALL, FIM_3DVIEW | FIM_3DMFMENU);
  AppendFuncItems("球面相机", GT_MSG_3DVIEW_SPHERECAMERA,
                  FIM_3DVIEW | FIM_3DMFMENU);
  AppendFuncItems("第一视角", GT_MSG_3DVIEW_FIRSTPERSON,
                  FIM_3DVIEW | FIM_3DMFMENU);
  AppendFuncItems("复位", GT_MSG_3DVIEW_RESTORE, FIM_3DVIEW | FIM_3DMFMENU);
  // AppendFuncItems("二维视图",GT_MSG_VIEW_ACTIVE,FIM_3DVIEW);

  IATOOL_APPEND_MSG(GT_MSG_3DVIEW_TRACEBALL);
  IATOOL_APPEND_MSG(GT_MSG_3DVIEW_SPHERECAMERA);
  IATOOL_APPEND_MSG(GT_MSG_3DVIEW_FIRSTPERSON);
  // IATOOL_APPEND_MSG(GT_MSG_3DVIEW_ACTIVE);

  IATOOL_APPEND_MSG(GT_MSG_3DVIEW_RESIZE);
  IATOOL_APPEND_MSG(GT_MSG_SET_3DVIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_GET_3DVIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_3DVIEW_RESTORE)

  RegisterMessage();

  return ERR_NONE;
}

int 3DViewCtrlTool ::AuxDraw() { return ERR_NONE; }

int 3DViewCtrlTool ::Timer() { return ERR_NONE; }

int 3DViewCtrlTool ::Notify(long nMessage, ListenerMessage& param) {
  if (param.source_window != m_hWnd) {
    switch (nMessage) {
      case GT_MSG_3DVIEW_ACTIVE: {
        SetForegroundWindow(m_hWnd);
        SetActive();
      } break;
      case GT_MSG_3DVIEW_RESTORE: {
        if (m_pScene) {
          Viewport3D viewport = m_p3DRenderDevice->GetViewport();

          /*	Aabb aabb = m_pScene->GetAabb();
          m_vOrgTarget= (aabb.vcMax + aabb.vcMin)/2.;
          m_vOrgEye.Set(m_vOrgTarget.x,m_vOrgTarget.y,aabb.vcMin.z);
          m_vOrgUp	= Vector3(0.f, 1.f, 0.f);*/

          Aabb aabb = m_pScene->GetAabb();
          m_vOrgEye = aabb.vcMax;
          m_vOrgTarget = (aabb.vcMax + aabb.vcMin) / 2.;
          m_vOrgUp = Vector3(0.f, 1.f, 0.f);

          m_pCamera->SetViewport(viewport);
          m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
          m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 100);
        }
      } break;
    }
  } else {
    switch (nMessage) {
      case GT_MSG_3DVIEW_TRACEBALL: {
        // m_viewMode = V3DM_TraceBall;
      } break;
      case GT_MSG_3DVIEW_SPHERECAMERA: {
        if (m_viewMode != V3DM_ShpereCamera) {
          SAFE_DELETE(m_pCamera);
          Viewport3D viewport = m_p3DRenderDevice->GetViewport();
          Aabb aabb = m_pScene->GetAabb();

          m_pCamera = new ArbvCamera(m_p3DRenderDevice, viewport);
          m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
          m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 100);

          if (m_pScene) m_pScene->SetSceneCamera(m_pCamera);

          m_viewMode = V3DM_ShpereCamera;
        }
      } break;
      case GT_MSG_3DVIEW_FIRSTPERSON: {
        if (m_viewMode != V3DM_FirstPerson) {
          SAFE_DELETE(m_pCamera);
          RECT wndRect;
          lPoint point;
          GetWindowRect(m_hWnd, &wndRect);
          point.x = (wndRect.right + wndRect.left) >> 1;
          point.y = (wndRect.bottom + wndRect.top) >> 1;

          Viewport3D viewport = m_p3DRenderDevice->GetViewport();
          Aabb aabb = m_pScene->GetAabb();

          m_pCamera = new FPSCamera(m_p3DRenderDevice, viewport);
          m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
          m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 100);
          ((FPSCamera*)m_pCamera)->SetWinCenter(point);

          if (m_pScene) m_pScene->SetSceneCamera(m_pCamera);

          m_viewMode = V3DM_FirstPerson;
        }
      } break;
      case GT_MSG_3DVIEW_RESTORE: {
        if (m_pScene) {
          Viewport3D viewport = m_p3DRenderDevice->GetViewport();

          /*	Aabb aabb = m_pScene->GetAabb();
          m_vOrgTarget= (aabb.vcMax + aabb.vcMin)/2.;
          m_vOrgEye.Set(m_vOrgTarget.x,m_vOrgTarget.y,aabb.vcMin.z);
          m_vOrgUp	= Vector3(0.f, 1.f, 0.f);*/

          Aabb aabb = m_pScene->GetAabb();
          m_vOrgEye = aabb.vcMax;
          m_vOrgTarget = (aabb.vcMax + aabb.vcMin) / 2.;
          m_vOrgUp = Vector3(0.f, 1.f, 0.f);

          m_pCamera->SetViewport(viewport);
          m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
          m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 100);
        }
      } break;
      case GT_MSG_3DVIEW_RESIZE: {
        Viewport3D vp = m_p3DRenderDevice->GetViewport();
        if (vp.ulHeight > 0 && vp.ulWidth > 0) {
          m_nWinWidth = vp.ulWidth;
          m_nWinHeight = vp.ulHeight;
        }
      } break;
      case GT_MSG_SET_3DVIEW_MODE: {
        m_viewMode = eView3DMode(*(ushort*)param.wparam);
      } break;
      case GT_MSG_GET_3DVIEW_MODE: {
        *(ushort*)param.wparam = m_viewMode;
      } break;
      default:
        break;
    }
    SetActive();
  }
  return ERR_NONE;
}

int 3DViewCtrlTool ::SetCursor(void) {
  switch (m_viewMode) {  // Zoom mode select
    case V3DM_TraceBall:
      ::SetCursor(m_hCursors[CursorTraceBall]);
      break;
    case V3DM_ShpereCamera:
      ::SetCursor(m_hCursors[CursorSphereCamare]);
      break;
    case V3DM_FirstPerson:
      ::SetCursor(m_hCursors[CursorFirstPerson]);
      break;
    default:
      // All other zoom modes
      ::SetCursor(m_hCrossCursor);
      break;
  }
  return ERR_NONE;
}

int 3DViewCtrlTool ::LButtonDown(uint nFlags, lPoint point) {
  m_bIsDrag = true;
  m_pntOrigin = m_pntCur = m_pntPre = point;
  if (m_viewMode == V3DM_TraceBall) {
    TrackballProv(point, m_vPrePos);
  }

  return ERR_NONE;
}

int 3DViewCtrlTool ::MouseMove(uint nFlags, lPoint point) {
  if (m_bIsDrag) {
    switch (m_viewMode) {
      case V3DM_ShpereCamera: {
        float deltx, delty;
        deltx = point.x - m_pntPre.x;
        delty = m_pntPre.y - point.y;

        ((ArbvCamera*)m_pCamera)->SetArbitMove(deltx, delty);

        m_pntPre = m_pntCur;
        m_pntCur = point;
      } break;
      case V3DM_TraceBall: {
        Vector3 curPos, deltV;
        TrackballProv(point, curPos);
        deltV = (curPos - m_vPrePos);

        if (deltV.x || deltV.y || deltV.z) {
          m_fAngle = 90.0f * deltV.GetLength();
          m_vAxis = m_vPrePos.CrossProduct(curPos);
          m_vPrePos = curPos;

          Matrix mat;
          mat.RotaArbi(m_vAxis, -m_fAngle * PI / 180.);

          m_pScene->TransWorld3DObjects(mat);
        }
      } break;
    }
  }

  if (m_viewMode == V3DM_FirstPerson) {
    ((FPSCamera*)m_pCamera)->SetViewByMouse();
  }
  return ERR_NONE;
}

int 3DViewCtrlTool ::LButtonUp(uint nFlags, lPoint point) {
  if (m_viewMode == V3DM_TraceBall) {
    m_fAngle = 0.0;
  }
  if (m_pLastTarget) {  //还原原来选择物体材质
    m_pLastTarget->SetMaterial(m_matLastTarget);
    m_pLastTarget = NULL;
  }

  if (point == m_pntOrigin) {
    //拾取模型
    m_vSelTargetPtrs.clear();
    m_pScene->Select3DObject(m_vSelTargetPtrs, point);

    //
    if (m_vSelTargetPtrs.size() > 0) {
      //更新材质
      m_pLastTarget = m_vSelTargetPtrs[0];
      m_matLastTarget = m_pLastTarget->GetMaterial();
      m_pLastTarget->SetMaterial(m_matSel);
    }
  }

  m_bIsDrag = false;

  return ERR_NONE;
}

int 3DViewCtrlTool ::KeyDown(uint nChar, uint nRepCnt, uint nFlags) {
  switch (nChar) {
    case 'W':
      m_pCamera->MoveForward();
      break;
    case 'S':
      m_pCamera->MoveBack();
      break;
    case 'A':
      m_pCamera->MoveLeft();
      break;
    case 'D':
      m_pCamera->MoveRight();
      break;
    case VK_UP:
      m_pCamera->MoveUp();
      break;
    case VK_DOWN:
      m_pCamera->MoveDown();
      break;
    case VK_LEFT:
      m_pCamera->Roll(PI / 120);
      break;
    case VK_RIGHT:
      m_pCamera->Roll(-PI / 120);
      break;
    case VK_PRIOR:
      m_pCamera->SetMoveStep(m_pCamera->GetMoveStep() + 10);
      break;
    case VK_NEXT:
      m_pCamera->SetMoveStep(m_pCamera->GetMoveStep() - 10);
      break;
    case 'K':
      m_p3DRenderDevice->SetShadeMode(RSV_SHADE_TRIWIRE, 0,
                                      Color(0., 1., 0., 1.0));
      break;
    case 'J':
      m_p3DRenderDevice->SetShadeMode(RSV_SHADE_SOLID, 0,
                                      Color(0., 1., 0., 1.0));
      break;
    case 'F':
      m_p3DRenderDevice->SetFog(FM_LINEAR, Color(0.8, 0.8, 0.8), 0.1, 30, 100);
      break;
    case 'G':
      m_p3DRenderDevice->SetFog(FM_NONE, Color(0.8, 0.8, 0.8), 0.1, 30, 100);
      break;
    case 'C': {
      ListenerMessage param;
      param.source_window = m_hWnd;
      Notify(GT_MSG_3DVIEW_TRACEBALL, param);
    } break;
    case 'V': {
      ListenerMessage param;
      param.source_window = m_hWnd;
      Notify(GT_MSG_3DVIEW_SPHERECAMERA, param);
    } break;
    case 'B': {
      ListenerMessage param;
      param.source_window = m_hWnd;
      Notify(GT_MSG_3DVIEW_FIRSTPERSON, param);
    } break;
    case 'Z': {
      ListenerMessage param;
      param.source_window = m_hWnd;
      Notify(GT_MSG_3DVIEW_RESTORE, param);
    } break;
    case 'O': {
      m_pScene->SetShowNodeBox(!m_pScene->IsShowNodeBox());
    }

    break;
    case VK_ESCAPE: {
      if (m_viewMode != V3DM_Normal) {
        SAFE_DELETE(m_pCamera);
        Viewport3D viewport = m_p3DRenderDevice->GetViewport();
        Aabb aabb = m_pScene->GetAabb();

        m_pCamera = new PerspCamera(m_p3DRenderDevice, viewport);
        m_pCamera->SetETU(m_vOrgEye, m_vOrgTarget, m_vOrgUp);
        m_pCamera->SetMoveStep((aabb.vcMax - aabb.vcMin).GetLength() / 500);

        m_pScene->SetSceneCamera(m_pCamera);

        m_viewMode = V3DM_Normal;
      }
    }

    break;
  }

  return ERR_NONE;
}

int 3DViewCtrlTool ::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  //方法一
  /*Viewport3D viewPort = m_p3DRenderDevice->GetViewport();
  viewPort.Fovy = 180.0/PI*(atan(0.1*x)+0.5*PI);;
  m_p3DRenderDevice->SetViewport(viewPort);
  m_p3DRenderDevice->MatrixModeSet(MM_PROJECTION);
  m_p3DRenderDevice->MatrixLoadIdentity();
  m_p3DRenderDevice->SetPerspective(viewPort.Fovy,(float)viewPort.Width/(float)viewPort.Height,viewPort.ZNear,viewPort.ZFar);
  m_p3DRenderDevice->MatrixModeSet(MM_MODELVIEW);
  m_p3DRenderDevice->MatrixLoadIdentity();*/

  //方法二
  m_pCamera->MoveEyeSmoothly(zDelta < 0);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
//根据二维的坐标x，y产生一组三维的坐标，单位化后存放到v中
void 3DViewCtrlTool ::TrackballProv(lPoint pos, Vector3& vec) {
  float d;
  vec.x = (2.0f * pos.x - m_nWinWidth) / m_nWinWidth;
  vec.y = (m_nWinHeight - 2.0f * pos.y) / m_nWinHeight;
  d = (float)sqrt(vec.x * vec.x + vec.y * vec.y);
  vec.z = (float)cos((PI / 2.0F) * ((d < 1.0) ? d : 1.0f));
  vec.Normalize();
}
}  // namespace _GroupTool
