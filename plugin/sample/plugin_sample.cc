// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#define MVIEW_PLUGIN_IMPLEMENTATION
#include "plugin/api/mview_plugin_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {

struct SampleUser {
  char id[64];
};

const char* SampleId(void* user) {
  SampleUser* self = static_cast<SampleUser*>(user);
  return self ? self->id : "plugin.sample.interaction";
}

int SampleOnInput(void* /*user*/, const MviewPluginInputEvent* /*event*/) {
  return 0;
}

void SampleDestroy(void* user) {
  free(user);
}

MviewPluginInteraction* SampleCreate(void* /*user_data*/) {
  MviewPluginInteraction* interaction = static_cast<MviewPluginInteraction*>(
      malloc(sizeof(MviewPluginInteraction)));
  if (!interaction)
    return NULL;
  SampleUser* user = static_cast<SampleUser*>(malloc(sizeof(SampleUser)));
  if (!user) {
    free(interaction);
    return NULL;
  }
  memset(user, 0, sizeof(*user));
  strncpy(user->id, "plugin.sample.interaction", sizeof(user->id) - 1);
  memset(interaction, 0, sizeof(*interaction));
  interaction->user = user;
  interaction->id = &SampleId;
  interaction->on_input = &SampleOnInput;
  interaction->destroy = &SampleDestroy;
  return interaction;
}

std::wstring ModuleDir() {
  HMODULE self = NULL;
  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(&ModuleDir), &self) ||
      !self) {
    return std::wstring();
  }
  std::wstring buf(32768, L'\0');
  DWORD n = GetModuleFileNameW(self, &buf[0], static_cast<DWORD>(buf.size()));
  if (n == 0 || n >= buf.size())
    return std::wstring();
  buf.resize(n);
  const size_t slash = buf.find_last_of(L"\\/");
  if (slash == std::wstring::npos)
    return std::wstring();
  return buf.substr(0, slash);
}

std::wstring ParentDir(const std::wstring& path) {
  const size_t slash = path.find_last_of(L"\\/");
  if (slash == std::wstring::npos || slash == 0)
    return std::wstring();
  return path.substr(0, slash);
}

bool FileExists(const std::wstring& path) {
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool IsRepoRoot(const std::wstring& dir) {
  return FileExists(dir + L"\\BUILD.gn") &&
         FileExists(dir + L"\\plugin\\sample\\plugin.json");
}

std::wstring FindRepoRoot() {
  std::wstring dir = ModuleDir();
  for (int i = 0; i < 8 && !dir.empty(); ++i) {
    if (IsRepoRoot(dir))
      return dir;
    const std::wstring parent = ParentDir(dir);
    if (parent.empty() || parent == dir)
      break;
    dir = parent;
  }
  return std::wstring();
}

bool EnsureDirectory(const std::wstring& path) {
  if (path.empty())
    return false;
  const DWORD attr = GetFileAttributesW(path.c_str());
  if (attr != INVALID_FILE_ATTRIBUTES)
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  const size_t slash = path.find_last_of(L"\\/");
  if (slash != std::wstring::npos && slash > 0) {
    if (!EnsureDirectory(path.substr(0, slash)))
      return false;
  }
  if (CreateDirectoryW(path.c_str(), NULL))
    return true;
  return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool WriteMarkFile() {
  const std::wstring root = FindRepoRoot();
  if (root.empty())
    return false;
  const std::wstring dir = root + L"\\out\\harness\\plugin.sample";
  if (!EnsureDirectory(dir))
    return false;
  const std::wstring path = dir + L"\\mark.json";
  const char* text =
      "{\n"
      "  \"command\": \"plugin.sample.run\",\n"
      "  \"marks\": [\"plugin.sample.ran\"]\n"
      "}\n";
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  DWORD written = 0;
  const BOOL ok =
      WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, NULL);
  CloseHandle(file);
  return ok == TRUE && written == strlen(text);
}

}  // namespace

extern "C" {

int mview_plugin_contribute(MviewPluginApi* api) {
  if (!api || api->version != MVIEW_PLUGIN_API_VERSION || !api->host)
    return 1;
  if (!api->contribute_command ||
      !api->contribute_interaction_factory || !api->contribute_menu_item ||
      !api->contribute_dock_panel)
    return 1;
  if (!api->contribute_command(api->host, "plugin.sample.run"))
    return 1;
  if (!api->contribute_interaction_factory(api->host, "plugin.sample.interaction",
                                           &SampleCreate, NULL))
    return 1;
  if (!api->contribute_menu_item(api->host, "plugin.sample.menu", "Sample Run",
                                 "plugin.sample.run"))
    return 1;
  if (!api->contribute_dock_panel(api->host, "plugin.sample.dock", "Sample"))
    return 1;
  return 0;
}

int mview_plugin_execute(const char* command_id) {
  if (!command_id || strcmp(command_id, "plugin.sample.run") != 0)
    return 1;
  return WriteMarkFile() ? 0 : 1;
}

}  // extern "C"
