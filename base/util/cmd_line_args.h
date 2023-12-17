// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_UTIL_CMD_LINE_ARGS_H_
#define BASE_UTIL_CMD_LINE_ARGS_H_

#include <ctype.h>

#include <vector>

#include "base/base_export.h"

namespace base {

class BASE_EXPORT CmdLineArgs : public std::vector<wchar_t *> {
 public:
  CmdLineArgs(const wchar_t *command_line = 0);
  virtual ~CmdLineArgs();

 private:
  bool ParseCommandLine();

  wchar_t *buffer_;
};

}  // namespace base

#endif  // BASE_UTIL_CMD_LINE_ARGS_H_
