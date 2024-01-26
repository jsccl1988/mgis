// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content_impl/factory/map_content_factory.h"

#include <map>

#include "base/logging.h"
#include "base/path/base_paths.h"
#include "base/util/string_util.h"
#include "content/public/map_content.h"
#include "gui/base/command_line.h"

namespace gui {
static const base::PathString kMapContentDLL = L"content.dll";

MapContentFactory::MapContentFactory() {}
MapContentFactory::~MapContentFactory() {}

bool MapContentFactory::CreateMapContent(content::MapContent** map_content) {
  if (!map_content) {
    return false;
  }

  CommandLine command_line;
  command_line.ParseFromString(::GetCommandLineW());

  base::PathString path = command_line.GetSwitchValue(CommandLine::kInstall);
  if (path.empty()) {
    base::PathProvider(base::DIR_EXE, &path);
  }

  path += L"\\";
  base::DirectoryAsDLLSearchPath(path + L"third_party\\bin");

  path += kMapContentDLL;

  HINSTANCE hInst = NULL;
  hInst = ::GetModuleHandle(path.c_str());

  {
    if (!hInst) {
      hInst =
          ::LoadLibraryEx(path.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }
  }

  if (!hInst) {
    LOG(ERROR) << "LoadLibrary,Last Error " << ::GetLastError()
               << base::UTF16ToUTF8(path);
    return false;
  }

  CreateMapContentFn pfnCreateMapContent = (CreateMapContentFn)GetProcAddress(
      hInst, content::kCreateMapContentFnName.c_str());

  if (!pfnCreateMapContent || FAILED(pfnCreateMapContent(map_content)) ||
      !(*map_content)) {
    return false;
  }

  return true;
}
}  // namespace gui
