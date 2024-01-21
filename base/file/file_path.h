// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_FILE_FILE_PATH_H_
#define BASE_FILE_FILE_PATH_H_

#include <string>

#include "base/base_config.h"

namespace base {
#if defined(OS_POSIX)
typedef std::string PathString;
#elif defined(OS_WIN)
typedef std::wstring PathString;
#endif

typedef PathString::value_type PathChar;
}  // namespace base

#endif  // BASE_FILE_FILE_PATH_H_
