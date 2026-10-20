// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef EXAMPLES_UI_DESIGNER_IO_H_
#define EXAMPLES_UI_DESIGNER_IO_H_

#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ui_designer {

std::wstring Utf8ToWidePath(const std::string& utf8);
std::string WideToUtf8(const std::wstring& wide);
std::string DirnameOf(const std::string& path);
std::string BasenameOf(const std::string& path);
std::string ToLowerCopy(const std::string& text);
bool ReadWriteTime(const std::string& path, FILETIME* out);
std::string ReadFileUtf8(const std::string& path);

}  // namespace ui_designer

#endif  // EXAMPLES_UI_DESIGNER_IO_H_
