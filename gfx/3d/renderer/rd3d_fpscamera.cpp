#include "rd3d_3drenderdevice.h"
#include "rd3d_camera.h"


namespace _3Drd {
FPSCamera::FPSCamera(LP3DRENDERDEVICE p3DRenderDevice, Viewport3D &viewport)
    : PerspCamera(p3DRenderDevice, viewport) {
  ;
}

FPSCamera::~FPSCamera(void) { ; }

void FPSCamera::SetViewByMouse(void) {
  POINT mousePos;

  GetCursorPos(&mousePos);

  if ((mousePos.x == m_winCenter.x) && (mousePos.y == m_winCenter.y)) return;

  SetCursorPos(m_winCenter.x, m_winCenter.y);

  float angleY = 0.0f;
  float angleZ = 0.0f;

  angleY = (float)((m_winCenter.x - mousePos.x)) / 1000.0f;
  angleZ = (float)((m_winCenter.y - mousePos.y)) / 1000.0f;

  if (angleY > 1.0) {
    angleY = 1.0;
    return;
  }

  if (angleY < -1.0) {
    angleY = -1.0;
    return;
  }

  Pitch(angleZ);
  Yaw(angleY);
}
}  // namespace _3Drd