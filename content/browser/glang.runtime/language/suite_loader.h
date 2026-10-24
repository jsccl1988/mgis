// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_GLANG_RUNTIME_LANGUAGE_SUITE_LOADER_H_
#define CONTENT_BROWSER_GLANG_RUNTIME_LANGUAGE_SUITE_LOADER_H_

#include <string>

namespace content {

// Reads a suite file as UTF-8 bytes. Strips a leading UTF-8 BOM.
// |error| may be NULL. Returns false when the path is empty, not valid
// UTF-8, or the file cannot be read. Does not interpret suite verbs.
bool load_suite_file(const std::string& path_utf8,
                     std::string* text,
                     std::string* error);

}  // namespace content

#endif  // CONTENT_BROWSER_GLANG_RUNTIME_LANGUAGE_SUITE_LOADER_H_
