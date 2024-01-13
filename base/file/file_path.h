// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_FILE_FILE_PATH_H_
#define BASE_FILE_FILE_PATH_H_

#include <string>

#include "base/base_config.h"

namespace base {
#if defined(OS_POSIX)
typedef std::string PathString;
typedef std::string NameString;
#elif defined(OS_WIN)
typedef std::wstring PathString;
typedef std::wstring NameString;
#endif

typedef PathString::value_type PathChar;
typedef NameString::value_type NameChar;
}  // namespace base

#endif  // BASE_FILE_FILE_PATH_H_
