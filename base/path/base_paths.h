// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PATH_BASE_PATHS_H
#define BASE_PATH_BASE_PATHS_H

// This file declares path keys for the base module.  These can be used with
// the PathService to access various special directories and files.

#include <string>

#include "base/file/file_path.h"
#include "base/path/base_paths_win.h"

namespace base {

enum BasePathKey {
  PATH_START = 0,

  DIR_CURRENT,       // Current directory.
  DIR_EXE,           // Directory containing FILE_EXE.
  DIR_MODULE,        // Directory containing FILE_MODULE.
  DIR_TEMP,          // Temporary directory.
  DIR_HOME,          // User's root home directory. On Windows this will look
                     // like "C:\Users\you" (or on XP
                     // "C:\Document and Settings\you") which isn't necessarily
                     // a great place to put files.
  FILE_EXE,          // Path and filename of the current executable.
  FILE_MODULE,       // Path and filename of the module containing the code for
                     // the PathService (which could differ from FILE_EXE if the
                     // PathService were compiled into a shared object, for
                     // example).
  FILE_MODULE_DIR,   // dir of FILE_MODULE.
  DIR_USER_DESKTOP,  // The current user's Desktop.

  DIR_TEST_DATA,  // Used only for testing.

  PATH_END
};

bool PathProvider(int key, PathString* result);
bool PathExists(const PathString& path);

bool DirectoryExists(const PathString& path);
bool DirectoryAsDLLSearchPath(const PathString& path);

bool GetCurrentDirectory(PathString* dir);
bool SetCurrentDirectory(const PathString& directory);

}  // namespace base

#endif  // BASE_PATH_BASE_PATHS_H
