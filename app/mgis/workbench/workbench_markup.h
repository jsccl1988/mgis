// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_WORKBENCH_WORKBENCH_MARKUP_H_
#define APP_MGIS_WORKBENCH_WORKBENCH_MARKUP_H_

#include <string_view>

namespace mgis {

// Embedded workbench.ui.xml with inline Style (same rules as workbench.ui.css).
// Used when the on-disk asset is not resolved next to the exe / source tree.
std::string_view WorkbenchMarkupXml();

}  // namespace mgis

#endif  // APP_MGIS_WORKBENCH_WORKBENCH_MARKUP_H_
