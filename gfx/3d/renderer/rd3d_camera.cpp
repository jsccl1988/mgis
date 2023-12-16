#include "rd3d_camera.h"

#include "rd3d_3drenderdevice.h"


namespace _3Drd {
Camera::Camera(LP3DRENDERDEVICE p3DRenderDevice, Viewport3D &viewport)
    : m_p3DRenderDevice(p3DRenderDevice), m_viewport(viewport) {
  ;
}

Camera::~Camera(void) { ; }

long Camera::Apply(void) {
  return m_p3DRenderDevice->GetStateManager()->SetViewportState(m_viewport);
}
}  // namespace _3Drd