// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/content_impl/map_content_factory.h"

#include <map>

#include "base/logging.h"
#include "base/util/string_util.h"

#include "content/public/map_box/map_box.h"
#include "content/public/map_content.h"
#include "gui/base/command_line.h"
#include "gui/base/path/base_paths.h"

namespace gui {
static const string16 kMapContentDLL = L"map_content.dll";

MapContentFactory::MapContentFactory() {}
MapContentFactory::~MapContentFactory() {}

bool MapContentFactory::CreateMapContent(content::MapContent** map_content) {
  if (!map_content) {
    return false;
  }

  CommandLine command_line;
  command_line.ParseFromString(::GetCommandLineW());

  string16 path = command_line.GetSwitchValue(CommandLine::kInstall);
  if (path.empty()) {
    PathProvider(DIR_EXE, &path);
  }

  path += L"\\wrs\\";
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
    LOG(ERROR) << "LoadLibrary,Last Error " << ::GetLastError() << base::UTF16ToUTF8(path);
    return false;
  }

  content::CreateMapContentFnPtr pfnCreateMapContent =
      (content::CreateMapContentFnPtr)GetProcAddress(
          hInst, content::CreateMapContentFnName);

  if (!pfnCreateMapContent || FAILED(pfnCreateMapContent(map_content)) ||
      !(*map_content))
    return false;

  return true;
}
}  // namespace gui
