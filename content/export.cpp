// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.
#include "base/base_config.h"
#include "base/path/base_paths.h"

#include "content/initializer.h"
#include "content/impl/map_content_impl.h"

namespace {
content::Initializer initiaizer;
base::ScopedRefPtr<content::MapContent> g_map_content;
}  // namespace

extern "C" {
int32_t CreateMapContent(content::MapContent** map_content) {
  if (!map_content) {
    return ERR_FAILURE;
  }

  *map_content = new content::MapContentImpl();
  g_map_content = *map_content;

  return ERR_NONE;
}

int32_t DestroyMapContent(content::MapContent** map_content) {
  if (!map_content) {
    return ERR_FAILURE;
  }

  SAFE_DELETE(map_content);
  return ERR_NONE;
}
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason,
                               LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH: {
      base::PathString module_dir;
      base::PathProvider(base::FILE_MODULE_DIR, &module_dir);
      base::DirectoryAsDLLSearchPath(module_dir);
      _Module.Init(NULL, ::GetModuleHandle(NULL));
      return TRUE;
    } break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
  }

  return FALSE;
}
