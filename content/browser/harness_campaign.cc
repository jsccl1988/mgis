// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/harness_campaign.h"

#include <string>
#include <vector>

#include <windows.h>

#include "base/file_util.h"
#include "content/browser/viewport/map/fake_scene.h"
#include "content/browser/glang.runtime/script_runtime.h"
#include "content/public/browser/types.h"

namespace content {
namespace {

std::string EscapeMark(const std::string& name) {
  std::string out;
  for (size_t i = 0; i < name.size(); ++i) {
    const char c = name[i];
    if (c == '\\' || c == '"')
      out.push_back('\\');
    out.push_back(c);
  }
  return out;
}

std::string MarksJson(const std::vector<std::string>& marks) {
  std::string json = "[";
  for (size_t i = 0; i < marks.size(); ++i) {
    if (i)
      json += ",";
    json += "\"";
    json += EscapeMark(marks[i]);
    json += "\"";
  }
  json += "]";
  return json;
}

std::string TrimLine(const std::string& line) {
  size_t begin = 0;
  while (begin < line.size() &&
         (line[begin] == ' ' || line[begin] == '\t' || line[begin] == '\r'))
    ++begin;
  size_t end = line.size();
  while (end > begin && (line[end - 1] == ' ' || line[end - 1] == '\t' ||
                         line[end - 1] == '\r'))
    --end;
  return line.substr(begin, end - begin);
}

bool ReadRequired(const base::FilePath& path, std::vector<std::string>* out) {
  std::string text;
  if (!file_util::ReadFileToString(path, &text))
    return false;
  size_t index = 0;
  while (index < text.size()) {
    const size_t newline = text.find('\n', index);
    const size_t end = newline == std::string::npos ? text.size() : newline;
    const std::string line = TrimLine(text.substr(index, end - index));
    if (!line.empty())
      out->push_back(line);
    if (newline == std::string::npos)
      break;
    index = end + 1;
  }
  return true;
}

bool WriteMarks(const base::FilePath& out_root,
                const char* suite,
                const std::vector<std::string>& marks) {
  const base::FilePath dir =
      out_root.AppendASCII("harness").AppendASCII(suite);
  if (!file_util::CreateDirectory(dir))
    return false;
  const std::string json = MarksJson(marks);
  return file_util::WriteFile(dir.AppendASCII("mark.json"), json.data(),
                              static_cast<int>(json.size())) ==
         static_cast<int>(json.size());
}

// One suite's FakeScene. The parent HWND outlives this object so the child
// window can be destroyed first.
struct SceneDriver {
  HWND parent;
  FakeScene scene;
  CapabilityHost* host;

  explicit SceneDriver(HWND parent_hwnd) : parent(parent_hwnd), host(NULL) {}
};

bool OnAttach(void* context, const std::string& view_id, std::string* error) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  const bool ok =
      driver->scene.Attach(driver->parent, view_id, 64, 64, NULL);
  if (!ok && error)
    *error = "attach failed";
  return ok;
}

bool OnSceneMode(void* context, const std::string& mode) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  return driver->scene.SetModeLabel(mode, NULL);
}

bool OnExpectCamera(void* context,
                    const std::string& view_id,
                    bool has_target,
                    double lon,
                    double lat,
                    double height,
                    std::string* error) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  if (view_id != kViewIdScene) {
    if (error)
      *error = "expect_camera accepts only scene";
    return false;
  }
  return driver->scene.ExpectCamera(has_target, lon, lat, height, error);
}

bool OnActivate(void* context,
                const std::string& command_id,
                const std::string& view_id) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  return driver->scene.ActivateTool(command_id, view_id);
}

bool OnCommand(void* context,
               const std::string& command_id,
               const std::string& view_id) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  if (command_id != "commit" || view_id != kViewIdScene)
    return false;
  std::string mark;
  if (!driver->scene.Commit(&mark))
    return false;
  if (!mark.empty())
    driver->host->mark(mark);
  return true;
}

bool OnPointer(void* context, const InputEvent& event) {
  SceneDriver* driver = static_cast<SceneDriver*>(context);
  std::string mark;
  if (!driver->scene.OnPointer(event, &mark))
    return false;
  if (!mark.empty())
    driver->host->mark(mark);
  return true;
}

bool RunSuite(const base::FilePath& suite_root,
              const base::FilePath& out_root,
              const char* name,
              HWND parent) {
  const base::FilePath dir = suite_root.AppendASCII(name);
  std::vector<std::string> required;
  if (!ReadRequired(dir.AppendASCII("required_marks.txt"), &required))
    return false;

  SceneDriver driver(parent);
  CapabilityHost host;
  driver.host = &host;
  host.set_attach_slot(&OnAttach, &driver);
  host.set_scene_mode_slot(&OnSceneMode, &driver);
  host.set_camera_slot(&OnExpectCamera, &driver);
  host.set_activate_slot(&OnActivate, &driver);
  host.set_command_slot(&OnCommand, &driver);
  host.set_pointer_slot(&OnPointer, &driver);

  std::string error;
  const base::FilePath suite = dir.AppendASCII("suite.txt");
  if (!run_suite_script(host, suite.AsUTF8Unsafe(), &error))
    return false;
  for (size_t i = 0; i < required.size(); ++i) {
    if (!host.has_mark(required[i]))
      return false;
  }
  return WriteMarks(out_root, name, host.marks());
}

struct ParentWindow {
  HWND hwnd;

  ParentWindow() {
    hwnd = CreateWindowExW(0, L"STATIC", L"mview-harness", WS_OVERLAPPED, 0, 0,
                           200, 120, NULL, NULL, GetModuleHandleW(NULL), NULL);
  }

  ~ParentWindow() {
    if (hwnd)
      DestroyWindow(hwnd);
  }
};

}  // namespace

bool HarnessCampaign::Run(const base::FilePath& suite_root,
                          const base::FilePath& out_root) {
  static const char* kSuites[] = {
      "browser.map",
      "browser.pick",
      "browser.draw",
  };
  ParentWindow parent;
  if (!parent.hwnd)
    return false;
  for (size_t i = 0; i < sizeof(kSuites) / sizeof(kSuites[0]); ++i) {
    if (!RunSuite(suite_root, out_root, kSuites[i], parent.hwnd))
      return false;
  }
  return true;
}

}  // namespace content
