// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BASE_COMMAND_LINE_H_
#define GUI_BASE_COMMAND_LINE_H_

#include <map>
#include <vector>

#include "base/basictypes.h"
#include "base/platform.h"

namespace gui {
class CommandLine {
 public:
  explicit CommandLine(const string16& program = string16(L""));

 public:
  void ParseFromString(const string16& command_line);

  const string16& GetCommandLine();
  string16 GetProgram() const { return program_; }
  const std::vector<string16>& GetNoneSwitchArgs() const {
    return nonswitch_args_;
  }

  string16 GetSwitchValue(const string16& switch_name);

  void AppendSwitchArg(const string16& switch_name,
                       const string16& switch_value);

  bool HasExternalLink() const;

 public:
  static const wchar_t* kPrefix;
  static const wchar_t* kDelimeter;
  static const wchar_t* kInstall;

 private:
  string16 argv_;
  wchar_t path_[MAX_PATH];
  bool changed_;

  string16 program_;
  std::map<string16, string16> switch_values_;
  // Non-switch command-line arguments.
  std::vector<string16> nonswitch_args_;

  DISALLOW_COPY_AND_ASSIGN(CommandLine);
};
}  // namespace gui

#endif  // GUI_BASE_COMMAND_LINE_H_
