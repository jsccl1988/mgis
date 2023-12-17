// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/win32/win_util.h"

#if defined(OS_WIN)

namespace base {
namespace win32 {

bool MBCSToUnicode(const char *input, std::wstring &output, int code_page) {
  output.clear();
  int length = ::MultiByteToWideChar(code_page, 0, input, -1, NULL, 0);
  if (length <= 0) return false;
  output.resize(length - 1);
  if (output.empty()) return true;
  ::MultiByteToWideChar(code_page, 0, input, -1, &output[0],
                        static_cast<int>(output.size()));
  return true;
}

bool MBCSToUnicode(const std::string &input, std::wstring &output,
                   int code_page) {
  output.clear();
  int length = ::MultiByteToWideChar(code_page, 0, input.c_str(),
                                     static_cast<int>(input.size()), NULL, 0);
  output.resize(length);
  if (output.empty()) return true;
  ::MultiByteToWideChar(code_page, 0, input.c_str(),
                        static_cast<int>(input.size()), &output[0],
                        static_cast<int>(output.size()));
  return true;
}

bool UnicodeToMBCS(const wchar_t *input, std::string &output, int code_page) {
  output.clear();
  int length =
      ::WideCharToMultiByte(code_page, 0, input, -1, NULL, 0, NULL, NULL);
  if (length <= 0) return false;
  output.resize(length - 1);
  if (output.empty()) return true;
  ::WideCharToMultiByte(code_page, 0, input, length - 1, &output[0],
                        static_cast<int>(output.size()), NULL, NULL);
  return true;
}

bool UnicodeToMBCS(const std::wstring &input, std::string &output,
                   int code_page) {
  output.clear();
  int length = ::WideCharToMultiByte(code_page, 0, input.c_str(),
                                     static_cast<int>(input.size()), NULL, 0,
                                     NULL, NULL);
  output.resize(length);
  if (output.empty()) return true;
  ::WideCharToMultiByte(code_page, 0, input.c_str(),
                        static_cast<int>(input.size()), &output[0],
                        static_cast<int>(output.size()), NULL, NULL);
  return true;
}

}  // namespace win32
}  // namespace base

#endif  // OS_WIN
