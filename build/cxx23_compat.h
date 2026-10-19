// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// C++20 removes std::ostream << const wchar_t* (the overload is deleted).
// Windows paths in this snapshot are wchar_t and are streamed from LOG sites.

#ifndef BUILD_CXX23_COMPAT_H_
#define BUILD_CXX23_COMPAT_H_

#include <ostream>
#include <string>

#include <windows.h>

inline std::ostream& operator<<(std::ostream& out, const wchar_t* wide) {
  if (!wide) {
    return out << "(null)";
  }
  const int needed =
      WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
  if (needed <= 1) {
    return out;
  }
  std::string utf8(static_cast<size_t>(needed), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide, -1, &utf8[0], needed, NULL, NULL);
  if (!utf8.empty() && utf8[utf8.size() - 1] == '\0') {
    utf8.resize(utf8.size() - 1);
  }
  return out << utf8;
}

#endif  // BUILD_CXX23_COMPAT_H_
