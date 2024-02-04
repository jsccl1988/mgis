#pragma once
#include "base/base_export.h"

namespace base {

BASE_EXPORT void Initialize(int argc, char* argv[]);  // NOLINT(modernize-avoid-c-arrays)
BASE_EXPORT void Deinitialize();

}  // namespace base
