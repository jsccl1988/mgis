// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Install-layout launcher. Paths are relative to this executable:
// Engine/, UI/mview_shell.exe, UI/vista_map.dll, UI/vista_world.dll,
// Bin/mview_volume.dll.
// It does not start Unreal. --check-layout builds a temp tree, checks those
// paths, and writes out/harness/browser.install/mark.json.

#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {

std::wstring ModulePath() {
  std::wstring buf(32768, L'\0');
  DWORD n = GetModuleFileNameW(NULL, &buf[0], static_cast<DWORD>(buf.size()));
  if (n == 0 || n >= buf.size())
    return std::wstring();
  buf.resize(n);
  return buf;
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

bool DirExists(const std::wstring& path) {
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool IsRepoRoot(const std::wstring& dir) {
  return FileExists(dir + L"\\BUILD.gn") &&
         FileExists(dir + L"\\plugin\\sample\\plugin.json");
}

std::wstring FindRepoRootFrom(std::wstring dir) {
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

std::wstring RepoRoot() {
  const std::wstring from_exe = FindRepoRootFrom(ParentDir(ModulePath()));
  if (!from_exe.empty())
    return from_exe;
  wchar_t cwd[MAX_PATH];
  const DWORD n = GetCurrentDirectoryW(MAX_PATH, cwd);
  if (n == 0 || n >= MAX_PATH)
    return std::wstring();
  return FindRepoRootFrom(cwd);
}

std::string WideToUtf8(const std::wstring& wide) {
  if (wide.empty())
    return std::string();
  const int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                    static_cast<int>(wide.size()), NULL, 0,
                                    NULL, NULL);
  if (n <= 0)
    return std::string();
  std::string out(static_cast<size_t>(n), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                      &out[0], n, NULL, NULL);
  return out;
}

bool EnsureDirectory(const std::wstring& path) {
  if (path.empty())
    return false;
  if (DirExists(path))
    return true;
  const size_t slash = path.find_last_of(L"\\/");
  if (slash != std::wstring::npos && slash > 0) {
    if (!EnsureDirectory(path.substr(0, slash)))
      return false;
  }
  if (CreateDirectoryW(path.c_str(), NULL))
    return true;
  return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool TouchFile(const std::wstring& path) {
  if (!EnsureDirectory(ParentDir(path)))
    return false;
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  CloseHandle(file);
  return true;
}

bool WriteFileBytes(const std::wstring& path, const std::string& text) {
  if (!EnsureDirectory(ParentDir(path)))
    return false;
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  DWORD written = 0;
  BOOL ok = TRUE;
  if (!text.empty()) {
    ok = WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written,
                   NULL);
  }
  CloseHandle(file);
  return ok == TRUE && written == text.size();
}

bool ReadFileBytes(const std::wstring& path, std::string* out) {
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  LARGE_INTEGER size;
  if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 ||
      size.QuadPart > 1024 * 1024) {
    CloseHandle(file);
    return false;
  }
  out->assign(static_cast<size_t>(size.QuadPart), '\0');
  DWORD read = 0;
  BOOL ok = TRUE;
  if (!out->empty()) {
    ok = ReadFile(file, &(*out)[0], static_cast<DWORD>(out->size()), &read,
                  NULL);
  }
  CloseHandle(file);
  return ok == TRUE && read == out->size();
}

bool HasCheckLayoutFlag() {
  const wchar_t* cmd = GetCommandLineW();
  const wchar_t* flag = L"--check-layout";
  const size_t n = wcslen(flag);
  for (const wchar_t* p = cmd; *p != L'\0'; ++p) {
    if (wcsncmp(p, flag, n) == 0) {
      const wchar_t next = p[n];
      if (next == L'\0' || next == L' ' || next == L'\t' || next == L'"')
        return true;
    }
  }
  return false;
}

// Relative install paths the product expects beside the launcher.
bool LayoutComplete(const std::wstring& root) {
  return DirExists(root + L"\\Engine") &&
         FileExists(root + L"\\UI\\mview_shell.exe") &&
         FileExists(root + L"\\UI\\vista_map.dll") &&
         FileExists(root + L"\\UI\\vista_world.dll") &&
         FileExists(root + L"\\Bin\\mview_volume.dll");
}

std::string LayoutReport(const std::wstring& root) {
  std::string report = "signed=false\n";
  report += "root=" + WideToUtf8(root) + "\n";
  report += std::string("Engine=") +
            (DirExists(root + L"\\Engine") ? "present" : "missing") + "\n";
  report += std::string("UI/mview_shell.exe=") +
            (FileExists(root + L"\\UI\\mview_shell.exe") ? "present"
                                                         : "missing") +
            "\n";
  report += std::string("UI/vista_map.dll=") +
            (FileExists(root + L"\\UI\\vista_map.dll") ? "present" : "missing") +
            "\n";
  report += std::string("UI/vista_world.dll=") +
            (FileExists(root + L"\\UI\\vista_world.dll") ? "present"
                                                         : "missing") +
            "\n";
  report += std::string("Bin/mview_volume.dll=") +
            (FileExists(root + L"\\Bin\\mview_volume.dll") ? "present"
                                                           : "missing") +
            "\n";
  return report;
}

const char* kP1Marks[] = {
    "widget.main", "widget.map",  "widget.view0",
    "widget.view1", "tree.layer", "tree.entity",
};

bool SplitLines(const std::string& text, std::vector<std::string>* lines) {
  size_t start = 0;
  while (start < text.size()) {
    size_t end = text.find('\n', start);
    if (end == std::string::npos)
      end = text.size();
    std::string line = text.substr(start, end - start);
    if (!line.empty() && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);
    if (!line.empty() && line[0] != '#')
      lines->push_back(line);
    start = end + 1;
  }
  return !lines->empty();
}

bool FixtureHasP1Marks(const std::vector<std::string>& lines) {
  for (size_t i = 0; i < sizeof(kP1Marks) / sizeof(kP1Marks[0]); ++i) {
    bool found = false;
    for (size_t j = 0; j < lines.size(); ++j) {
      if (lines[j] == kP1Marks[i])
        found = true;
    }
    if (!found)
      return false;
  }
  return true;
}

std::string InstallMarkJson(const std::vector<std::string>& marks) {
  std::string json = "{\n  \"signed\": false,\n  \"marks\": [\n";
  for (size_t i = 0; i < marks.size(); ++i) {
    json += "    \"";
    json += marks[i];
    json += "\"";
    if (i + 1 != marks.size())
      json += ",";
    json += "\n";
  }
  json += "  ]\n}\n";
  return json;
}

bool CreateTempInstall(std::wstring* root) {
  wchar_t temp[MAX_PATH];
  const DWORD n = GetTempPathW(MAX_PATH, temp);
  if (n == 0 || n >= MAX_PATH)
    return false;
  wchar_t name[96];
  _snwprintf_s(name, sizeof(name) / sizeof(name[0]), _TRUNCATE,
               L"mview-layout-%lu-%lu",
               static_cast<unsigned long>(GetCurrentProcessId()),
               static_cast<unsigned long>(GetTickCount()));
  *root = std::wstring(temp) + name;
  if (!EnsureDirectory(*root))
    return false;
  if (!EnsureDirectory(*root + L"\\Engine"))
    return false;
  if (!TouchFile(*root + L"\\Engine\\placeholder.txt"))
    return false;
  if (!TouchFile(*root + L"\\UI\\mview_shell.exe"))
    return false;
  if (!TouchFile(*root + L"\\UI\\vista_map.dll"))
    return false;
  if (!TouchFile(*root + L"\\UI\\vista_world.dll"))
    return false;
  if (!TouchFile(*root + L"\\Bin\\mview_volume.dll"))
    return false;

  std::string fixture;
  for (size_t i = 0; i < sizeof(kP1Marks) / sizeof(kP1Marks[0]); ++i) {
    fixture += kP1Marks[i];
    fixture += "\n";
  }
  return WriteFileBytes(*root + L"\\UI\\p1_marks.txt", fixture);
}

int RunShell(const std::wstring& shell) {
  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));
  if (!CreateProcessW(shell.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL,
                      &si, &pi)) {
    return 1;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 1;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return static_cast<int>(code);
}

std::wstring ShellBesideOrDebug(const std::wstring& exe_dir,
                                const std::wstring& repo) {
  const std::wstring beside = exe_dir + L"\\mview_shell.exe";
  if (FileExists(beside))
    return beside;
  if (!repo.empty()) {
    const std::wstring debug = repo + L"\\out\\Debug\\mview_shell.exe";
    if (FileExists(debug))
      return debug;
  }
  return std::wstring();
}

int CheckLayout() {
  std::wstring temp_root;
  if (!CreateTempInstall(&temp_root)) {
    fprintf(stderr, "check-layout: failed to create temp install tree\n");
    return 1;
  }
  if (!LayoutComplete(temp_root)) {
    fprintf(stderr, "check-layout: relative paths missing under %s\n",
            WideToUtf8(temp_root).c_str());
    return 1;
  }

  std::string fixture;
  if (!ReadFileBytes(temp_root + L"\\UI\\p1_marks.txt", &fixture)) {
    fprintf(stderr, "check-layout: fixture missing\n");
    return 1;
  }
  std::vector<std::string> marks;
  if (!SplitLines(fixture, &marks) || !FixtureHasP1Marks(marks)) {
    fprintf(stderr, "check-layout: fixture is missing P1 marks\n");
    return 1;
  }
  marks.push_back("viewport.reattach");

  const std::string report = LayoutReport(temp_root);
  if (!WriteFileBytes(temp_root + L"\\layout-report.txt", report))
    return 1;
  fputs(report.c_str(), stdout);

  const std::wstring repo = RepoRoot();
  if (repo.empty()) {
    fprintf(stderr, "check-layout: repo root not found\n");
    return 1;
  }
  const std::wstring mark_path =
      repo + L"\\out\\harness\\browser.install\\mark.json";
  const std::string json = InstallMarkJson(marks);
  if (!WriteFileBytes(mark_path, json)) {
    fprintf(stderr, "check-layout: failed to write mark\n");
    return 1;
  }

  std::string written;
  if (!ReadFileBytes(mark_path, &written))
    return 1;
  if (written.find("\"signed\": false") == std::string::npos)
    return 1;
  if (written.find("\"signed\": true") != std::string::npos)
    return 1;
  for (size_t i = 0; i < marks.size(); ++i) {
    if (written.find(marks[i]) == std::string::npos)
      return 1;
  }
  return 0;
}

}  // namespace

int main() {
  const std::wstring exe_dir = ParentDir(ModulePath());
  if (HasCheckLayoutFlag())
    return CheckLayout();

  const std::wstring shell = ShellBesideOrDebug(exe_dir, RepoRoot());
  if (!shell.empty())
    return RunShell(shell);

  const std::string report = LayoutReport(exe_dir);
  WriteFileBytes(exe_dir + L"\\layout-report.txt", report);
  fputs(report.c_str(), stdout);
  return 0;
}
