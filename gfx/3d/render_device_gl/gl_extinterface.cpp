#include "gl_3drenderdevice.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reson_for_call,
                      LPVOID lpReserved) {
  switch (ul_reson_for_call) {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return true;
}

namespace _3Drd {

HRESULT Create3DRenderDevice(HINSTANCE hDLL, 3DRenderDevice * &pDevice) {
  if (!pDevice) {
    pDevice = new GLRenderDevice(hDLL);
    return OK;
  }

  return FALSE;
}

HRESULT Release3DRenderDevice(3DRenderDevice * &pDevice) {
  if (!pDevice) {
    return FALSE;
  }
  delete pDevice;
  pDevice = NULL;

  return OK;
}
}  // namespace _3Drd