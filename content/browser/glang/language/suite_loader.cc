// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/glang/language/suite_loader.h"

#include <stdio.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace content {
namespace {

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

#if defined(_WIN32)
bool Utf8ToWide(const std::string& utf8, std::wstring* wide) {
  if (utf8.empty()) {
    wide->clear();
    return true;
  }
  if (utf8.size() > 0x7fffffffu)
    return false;
  const int in_len = static_cast<int>(utf8.size());
  const int out_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          utf8.data(), in_len, NULL, 0);
  if (out_len <= 0)
    return false;
  wide->assign(static_cast<size_t>(out_len), L'\0');
  const int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          utf8.data(), in_len, &(*wide)[0],
                                          out_len);
  return written == out_len;
}
#endif

bool OpenSuite(const std::string& path_utf8, FILE** file, std::string* error) {
  *file = NULL;
#if defined(_WIN32)
  std::wstring wide;
  if (!Utf8ToWide(path_utf8, &wide)) {
    SetError(error, "suite path is not valid utf-8");
    return false;
  }
#if defined(_MSC_VER)
  if (_wfopen_s(file, wide.c_str(), L"rb") != 0 || *file == NULL) {
    *file = NULL;
    SetError(error, "failed to open suite: " + path_utf8);
    return false;
  }
#else
  *file = _wfopen(wide.c_str(), L"rb");
  if (*file == NULL) {
    SetError(error, "failed to open suite: " + path_utf8);
    return false;
  }
#endif
#else
  *file = fopen(path_utf8.c_str(), "rb");
  if (*file == NULL) {
    SetError(error, "failed to open suite: " + path_utf8);
    return false;
  }
#endif
  return true;
}

void StripBom(std::string* text) {
  if (text->size() >= 3 &&
      static_cast<unsigned char>((*text)[0]) == 0xef &&
      static_cast<unsigned char>((*text)[1]) == 0xbb &&
      static_cast<unsigned char>((*text)[2]) == 0xbf) {
    text->erase(0, 3);
  }
}

}  // namespace

bool load_suite_file(const std::string& path_utf8,
                     std::string* text,
                     std::string* error) {
  if (!text) {
    SetError(error, "suite text output is null");
    return false;
  }
  text->clear();
  if (path_utf8.empty()) {
    SetError(error, "suite path is empty");
    return false;
  }

  FILE* file = NULL;
  if (!OpenSuite(path_utf8, &file, error))
    return false;

  char buffer[4096];
  while (true) {
    const size_t n = fread(buffer, 1, sizeof(buffer), file);
    if (n > 0)
      text->append(buffer, n);
    if (n < sizeof(buffer)) {
      if (ferror(file)) {
        fclose(file);
        text->clear();
        SetError(error, "failed to read suite: " + path_utf8);
        return false;
      }
      break;
    }
  }
  fclose(file);
  StripBom(text);
  return true;
}

}  // namespace content
