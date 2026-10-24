// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/ui_designer/io.h"

#include <cstdio>

namespace ui_designer {

std::wstring Utf8ToWidePath(const std::string& utf8) {
  if (utf8.empty())
    return std::wstring();
  const int n = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
  if (n <= 0)
    return std::wstring();
  std::wstring wide(static_cast<size_t>(n), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], n);
  wide.resize(static_cast<size_t>(n - 1));
  return wide;
}

std::string WideToUtf8(const std::wstring& wide) {
  if (wide.empty())
    return std::string();
  const int n = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0,
                                    NULL, NULL);
  if (n <= 0)
    return std::string();
  std::string utf8(static_cast<size_t>(n), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], n, NULL, NULL);
  utf8.resize(static_cast<size_t>(n - 1));
  return utf8;
}

std::string DirnameOf(const std::string& path) {
  const size_t slash = path.find_last_of("\\/");
  if (slash == std::string::npos)
    return std::string();
  return path.substr(0, slash);
}

std::string BasenameOf(const std::string& path) {
  const size_t slash = path.find_last_of("\\/");
  if (slash == std::string::npos)
    return path;
  return path.substr(slash + 1);
}

std::string ToLowerCopy(const std::string& text) {
  std::string out = text;
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i] >= 'A' && out[i] <= 'Z')
      out[i] = static_cast<char>(out[i] - 'A' + 'a');
  }
  return out;
}

bool ReadWriteTime(const std::string& path, FILETIME* out) {
  if (!out)
    return false;
  const std::wstring wide = Utf8ToWidePath(path);
  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!GetFileAttributesExW(wide.c_str(), GetFileExInfoStandard, &data))
    return false;
  *out = data.ftLastWriteTime;
  return true;
}

std::string ReadFileUtf8(const std::string& path) {
  const std::wstring wide = Utf8ToWidePath(path);
  if (wide.empty())
    return std::string();
  FILE* file = NULL;
  if (_wfopen_s(&file, wide.c_str(), L"rb") != 0 || !file)
    return std::string();
  std::string data;
  char buf[4096];
  while (const size_t n = fread(buf, 1, sizeof(buf), file))
    data.append(buf, n);
  fclose(file);
  return data;
}

}  // namespace ui_designer
