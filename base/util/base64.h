// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_UTIL_BASE64_H_
#define BASE_UTIL_BASE64_H_

#include <string>

#include "base/base_export.h"

namespace base {
BASE_EXPORT bool Base64Encode(const std::string& input, std::string* output);
BASE_EXPORT bool Base64Decode(const std::string& input, std::string* output);
}  // namespace base

#endif  // BASE_UTIL_BASE64_H_
