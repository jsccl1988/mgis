#include "gl_3drenderdevice.h"

namespace _3Drd {
bool GLRenderDevice::IsExtensionSupported(string extension) {
  char* ext = (char*)glGetString(GL_EXTENSIONS);
  if (NULL != ext && strstr(ext, extension.c_str()) != NULL) {
    return true;
  }

  return false;
}

void* GLRenderDevice::GetProcAddress(string name) {
  return wglGetProcAddress(name.c_str());
}

GPUStateManager* GLRenderDevice::GetStateManager() { return m_pStateManager; }

3DDeviceCaps * GLRenderDevice::GetDeviceCaps() { return m_pDeviceCaps; }
}  // namespace _3Drd