#include "gl_devicecaps.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
GLDeviceCaps::GLDeviceCaps(LP3DRENDERDEVICE p3DRenderDevice) : 3DDeviceCaps(
    p3DRenderDevice) {
  ;
}

bool GLDeviceCaps::IsVSyncSupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
      ->IsExtensionSupported("WGL_EXT_swap_control");
}

bool GLDeviceCaps::IsAnisotropySupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
      ->IsExtensionSupported("GL_EXT_texture_filter_anisotropic");
}

bool GLDeviceCaps::IsVBOSupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
      ->IsExtensionSupported("GL_ARB_vertex_buffer_object");
}

bool GLDeviceCaps::IsMipMapsSupported() {
  return false;
  // return
  // ((GLRenderDevice*)m_p3DRenderDevice)->IsExtensionSupported("GL_SGIS_generate_mipmap");
}

bool GLDeviceCaps::IsFBOSupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
      ->IsExtensionSupported("EXT_framebuffer_object");
}

bool GLDeviceCaps::IsGLSLSupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
             ->IsExtensionSupported("GL_ARB_shading_language_100") &&
         ((GLRenderDevice*)m_p3DRenderDevice)
             ->IsExtensionSupported("GL_ARB_shader_objects") &&
         ((GLRenderDevice*)m_p3DRenderDevice)
             ->IsExtensionSupported("GL_ARB_vertex_shader") &&
         ((GLRenderDevice*)m_p3DRenderDevice)
             ->IsExtensionSupported("GL_ARB_fragment_shader");
}

bool GLDeviceCaps::IsMultiTextureSupported() {
  return ((GLRenderDevice*)m_p3DRenderDevice)
      ->IsExtensionSupported("ARB_multitexture");
}

int GLDeviceCaps::GetTextureSlotsCount() {
  int maxTextureUnits = 0;
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
  return maxTextureUnits;
}

int GLDeviceCaps::GetMaxColorAttachments() {
  // return FBOFunctions->getMaxColorAttachments();
  return -1;
}

float GLDeviceCaps::GetMaxAnisotropy() {
  float maxLevel = 0.0;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxLevel);
  return maxLevel;
}

}  // namespace _3Drd