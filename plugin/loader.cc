// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "plugin/loader.h"

#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "base/logging.h"
#include "content/public/browser/plugin_host.h"
#include "content/public/browser/trace_log.h"
#include "plugin/api/mview_plugin_api.h"

namespace plugin {
namespace {

typedef int (*ContributeFn)(MviewPluginApi* api);
typedef int (*ExecuteFn)(const char* command_id);

std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty())
    return std::wstring();
  const int in_len = static_cast<int>(utf8.size());
  const int out_len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), in_len, NULL,
                                          0);
  if (out_len <= 0)
    return std::wstring();
  std::wstring wide(static_cast<size_t>(out_len), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), in_len, &wide[0], out_len);
  return wide;
}

bool FileExistsUtf8(const std::string& path_utf8) {
  const std::wstring path = Utf8ToWide(path_utf8);
  if (path.empty())
    return false;
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::string JoinPath(const std::string& dir, const std::string& file) {
  if (dir.empty())
    return file;
  if (!dir.empty() && (dir[dir.size() - 1] == '\\' || dir[dir.size() - 1] == '/'))
    return dir + file;
  return dir + "\\" + file;
}

void TraceSkip(content::TraceLog* trace, const std::string& message) {
  LOG(WARNING) << message;
  if (trace)
    trace->Append(content::kTraceConsole, message);
}

struct HostShim {
  content::PluginHost* host;
  ExecuteFn execute;
};

struct CommandBind {
  ExecuteFn execute;
  char command_id[256];
};

int RunBoundCommand(void* user_data, char* error, int error_cap) {
  CommandBind* bind = static_cast<CommandBind*>(user_data);
  if (!bind || !bind->execute) {
    if (error && error_cap > 0) {
      strncpy(error, "no execute", static_cast<size_t>(error_cap - 1));
      error[error_cap - 1] = '\0';
    }
    return 1;
  }
  const int rc = bind->execute(bind->command_id);
  if (rc != 0 && error && error_cap > 0) {
    strncpy(error, "mview_plugin_execute failed",
            static_cast<size_t>(error_cap - 1));
    error[error_cap - 1] = '\0';
  }
  return rc;
}

int ContributeCommandCb(void* host, const char* command_id) {
  HostShim* shim = static_cast<HostShim*>(host);
  if (!shim || !shim->host || !command_id || !command_id[0])
    return 0;
  if (shim->execute) {
    CommandBind* bind = new CommandBind();
    bind->execute = shim->execute;
    strncpy(bind->command_id, command_id, sizeof(bind->command_id) - 1);
    bind->command_id[sizeof(bind->command_id) - 1] = '\0';
    return shim->host->ContributeCommand(command_id, &RunBoundCommand, bind) ? 1
                                                                             : 0;
  }
  return shim->host->contribute_command_id(command_id) ? 1 : 0;
}

int ContributeFactoryCb(void* host,
                        const char* factory_id,
                        MviewPluginCreateInteractionFn create,
                        void* user_data) {
  HostShim* shim = static_cast<HostShim*>(host);
  if (!shim || !shim->host || !factory_id)
    return 0;
  return shim->host->ContributeInteractionFactory(factory_id, create,
                                                  user_data)
             ? 1
             : 0;
}

int ContributeMenuCb(void* host,
                     const char* menu_id,
                     const char* label_utf8,
                     const char* command_id) {
  HostShim* shim = static_cast<HostShim*>(host);
  if (!shim || !shim->host)
    return 0;
  return shim->host->ContributeMenuItem(
             menu_id ? menu_id : "", label_utf8 ? label_utf8 : "",
             command_id ? command_id : "")
             ? 1
             : 0;
}

int ContributeDockCb(void* host,
                     const char* panel_id,
                     const char* title_utf8) {
  HostShim* shim = static_cast<HostShim*>(host);
  if (!shim || !shim->host)
    return 0;
  return shim->host->ContributeDockPanel(panel_id ? panel_id : "",
                                         title_utf8 ? title_utf8 : "")
             ? 1
             : 0;
}

int RunProcessingCb(void* host,
                    const char* algorithm_id,
                    char* error,
                    int error_cap) {
  HostShim* shim = static_cast<HostShim*>(host);
  if (!shim || !shim->host) {
    if (error && error_cap > 0) {
      strncpy(error, "no host", static_cast<size_t>(error_cap - 1));
      error[error_cap - 1] = '\0';
    }
    return 1;
  }
  std::string err;
  const bool ok =
      shim->host->run_processing(algorithm_id ? algorithm_id : "", &err);
  if (!ok && error && error_cap > 0) {
    strncpy(error, err.c_str(), static_cast<size_t>(error_cap - 1));
    error[error_cap - 1] = '\0';
  }
  return ok ? 0 : 1;
}

std::string ResolveDllPath(const Manifest& manifest,
                           const std::vector<std::string>& search_dirs) {
  if (manifest.dll.empty())
    return std::string();
  if (!manifest.directory.empty()) {
    const std::string candidate = JoinPath(manifest.directory, manifest.dll);
    if (FileExistsUtf8(candidate))
      return candidate;
  }
  for (size_t i = 0; i < search_dirs.size(); ++i) {
    const std::string candidate = JoinPath(search_dirs[i], manifest.dll);
    if (FileExistsUtf8(candidate))
      return candidate;
  }
  return std::string();
}

}  // namespace

Loader::Loader() {}

Loader::~Loader() {
  // Keep modules loaded for process lifetime. PluginHost may still hold
  // CommandRunFn / InteractionFactory pointers into the DLL after contribute.
  modules_.clear();
}

int Loader::LoadAll(const Registry& registry,
                    content::PluginHost* host,
                    const std::vector<std::string>& search_dirs,
                    content::TraceLog* trace) {
  if (!host)
    return 0;
  int loaded = 0;
  const std::vector<Manifest> list = registry.List();
  for (size_t i = 0; i < list.size(); ++i) {
    if (list[i].dll.empty())
      continue;
    const std::string path = ResolveDllPath(list[i], search_dirs);
    if (path.empty()) {
      TraceSkip(trace, "plugin.skip missing dll " + list[i].id + " " +
                           list[i].dll);
      continue;
    }
    if (LoadOne(list[i], path, host, trace))
      ++loaded;
  }
  return loaded;
}

bool Loader::LoadOne(const Manifest& manifest,
                     const std::string& dll_path_utf8,
                     content::PluginHost* host,
                     content::TraceLog* trace) {
  if (!host || dll_path_utf8.empty())
    return false;
  const std::wstring wide = Utf8ToWide(dll_path_utf8);
  if (wide.empty())
    return false;
  HMODULE module = LoadLibraryW(wide.c_str());
  if (!module) {
    TraceSkip(trace, "plugin.skip LoadLibrary " + manifest.id);
    return false;
  }
  ContributeFn contribute = reinterpret_cast<ContributeFn>(
      GetProcAddress(module, "mview_plugin_contribute"));
  if (!contribute) {
    TraceSkip(trace, "plugin.skip no contribute " + manifest.id);
    FreeLibrary(module);
    return false;
  }
  ExecuteFn execute = reinterpret_cast<ExecuteFn>(
      GetProcAddress(module, "mview_plugin_execute"));

  HostShim shim;
  shim.host = host;
  shim.execute = execute;

  MviewPluginApi api;
  memset(&api, 0, sizeof(api));
  api.version = MVIEW_PLUGIN_API_VERSION;
  api.host = &shim;
  api.contribute_command = &ContributeCommandCb;
  api.contribute_interaction_factory = &ContributeFactoryCb;
  api.contribute_menu_item = &ContributeMenuCb;
  api.contribute_dock_panel = &ContributeDockCb;
  api.run_processing = &RunProcessingCb;

  const int rc = contribute(&api);
  if (rc != 0) {
    TraceSkip(trace, "plugin.skip contribute " + manifest.id);
    FreeLibrary(module);
    return false;
  }

  modules_.push_back(module);
  if (trace)
    trace->Append(content::kTraceConsole, "plugin.loaded " + manifest.id);
  LOG(INFO) << "plugin loaded " << manifest.id << " from " << dll_path_utf8;
  return true;
}

}  // namespace plugin
