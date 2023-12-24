// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#include "gui/content/map_content_factory.h"

#include <map>

#include "gui/base/command_line.h"
#include "gui/base/path/base_paths.h"

#include "browser/map_box.h"
#include "content.h"

#include "third_party/bdlog/bdlog.h"

namespace gui {
static const string16 kMapContentProxyDLL = L"map_content.dll";

MapContentFactory::MapContentFactory() {}
MapContentFactory::~MapContentFactory() {}

bool MapContentFactory::CreateMapContent(content::MapContent ** content) {
  if (!content) {
    return false;
  }

  CommandLine command_line;
  command_line.ParseFromString(::GetCommandLineW());

  string16 path = command_line.GetSwitchValue(CommandLine::kInstall);
  if (path.empty()) {
    PathProvider(DIR_EXE, &path);
  }

  path += L"\\wrs\\";
  path += kMapContentProxyDLL;

  HINSTANCE hInst = NULL;
  hInst = ::GetModuleHandle(path.c_str());

  {
    if (!hInst) {
      hInst = ::LoadLibraryEx(path.c_str()
        , NULL
        , LOAD_WITH_ALTERED_SEARCH_PATH);
    }
  }

  if (!hInst) {
    Log(LL_DEBUG
      , TAG(L"Demo")
      , _T("LoadLibrary,Last Error %d,%s")
      , ::GetLastError(), path.c_str());

    return false;
  }

  content::CreateMapContentFnPtr pfnCreateMapContent =
    (content::CreateMapContentFnPtr) GetProcAddress(
      hInst, content::CreateMapContentFnName);

  if (!pfnCreateMapContent
    || FAILED(pfnCreateMapContent(content))
    || !(*content))
    return false;

  return true;
}
}  // namespace gui
