// Copyright (c) 2012 The Chromium Authors.
// All rights reserved.

#include "gui/base/command_line.h"

#include <assert.h>
#include <atlstr.h>
#include <shellapi.h>

#include <map>
#include <vector>

namespace gui {
const wchar_t* CommandLine::kPrefix = TEXT("--");
const wchar_t* CommandLine::kDelimeter = TEXT("=");

const wchar_t* CommandLine::kInstall = TEXT("install");

// static
static bool IsSwitch(const ATL::CString& parameter_string,
                     ATL::CString* switch_name, ATL::CString* switch_value) {
  if (NULL == switch_name || NULL == switch_value) {
    return false;
  }
  switch_name->Empty();
  switch_value->Empty();

  ATL::CString prefix = CommandLine::kPrefix;
  if (parameter_string.Find(prefix) != 0) return false;

  const int32_t switch_start = prefix.GetLength();
  const int32_t equals_psition =
      parameter_string.Find(CommandLine::kDelimeter, switch_start);

  if (equals_psition == -1) {
    *switch_name = parameter_string.Mid(switch_start);
  } else {
    *switch_name =
        parameter_string.Mid(switch_start, equals_psition - switch_start);
    *switch_value = parameter_string.Mid(equals_psition + 1);
    switch_value->Trim(L"\"");
  }

  switch_name->MakeLower();

  return true;
}

CommandLine::CommandLine(const string16& program) : changed_(false) {
  if (!program.empty()) {
    program_ = program;
  }

  path_[0] = 0;
}

const string16& CommandLine::GetCommandLine() {
  if (changed_ || argv_.empty()) {
    argv_ += L'"' + program_ + L'"';
    ATL::CString argv(argv_.c_str());
    for (std::map<string16, string16>::const_iterator iter =
             switch_values_.begin();
         iter != switch_values_.end(); ++iter) {
      argv.AppendFormat(TEXT(" %s%s%s\"%s\""), kPrefix, iter->first.c_str(),
                        kDelimeter, iter->second.c_str());
    }

    for (std::vector<string16>::const_iterator iter = nonswitch_args_.begin();
         iter != nonswitch_args_.end(); ++iter) {
      argv.AppendFormat(TEXT(" %s \"%s\""), kPrefix, (*iter).c_str());
    }

    argv.Trim();
    argv_ = argv;
    changed_ = false;
  }

  return argv_;
}

void CommandLine::ParseFromString(const string16& command_line) {
  ATL::CString command_line_copy = command_line.c_str();
  command_line_copy.Trim();

  if (command_line_copy.IsEmpty()) return;

  int32_t args_number = 0;
  wchar_t** args = NULL;

  args = ::CommandLineToArgvW(command_line_copy, &args_number);

  // Populate program_ with the trimmed version of the first arg.
  ATL::CString program = args[0];
  program_ = program.Trim();

  bool parse_switches = true;
  for (int32_t i = 1; i < args_number; ++i) {
    ATL::CString arg = args[i];
    arg.Trim();

    if (!parse_switches) {
      nonswitch_args_.push_back(string16(arg));
      continue;
    }

    if (arg == kPrefix) {
      parse_switches = false;
      continue;
    }

    ATL::CString switch_name;
    ATL::CString switch_value;
    if (IsSwitch(arg, &switch_name, &switch_value)) {
      switch_values_[string16(switch_name)] = switch_value;
    } else {
      nonswitch_args_.push_back(string16(arg));
    }
  }

  if (args) {
    LocalFree(args);
  }

  changed_ = true;
}

string16 CommandLine::GetSwitchValue(const string16& switch_name) {
  string16 switch_value;
  ATL::CString lower_case_switch_name = switch_name.c_str();
  lower_case_switch_name.MakeLower();

  if (switch_values_.count(string16(lower_case_switch_name)) > 0)
    switch_value = switch_values_[string16(lower_case_switch_name)];

  return switch_value;
}

void CommandLine::AppendSwitchArg(const string16& switch_name,
                                  const string16& switch_value) {
  assert(!switch_name.empty());
  assert(!switch_value.empty());

  switch_values_[switch_name] = switch_value;
  changed_ = true;
}

bool CommandLine::HasExternalLink() const { return !nonswitch_args_.empty(); }
}  // namespace gui
