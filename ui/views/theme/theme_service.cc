// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/theme/theme_service.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace views {
namespace {

// Prefer %LOCALAPPDATA%\MView over SHGetKnownFolderPath to avoid pulling
// shell headers that break MSVC c++23preview chrono/ratio in this TU.
std::wstring ThemePersistDir() {
  wchar_t folder[MAX_PATH] = {};
  const DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", folder, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return std::wstring();
  return std::wstring(folder) + L"\\MView";
}

std::wstring ThemePersistFile() {
  std::wstring dir = ThemePersistDir();
  if (dir.empty())
    return std::wstring();
  return dir + L"\\ui_theme_id.txt";
}

}  // namespace

ThemeService& ThemeService::Get() {
  static ThemeService instance;
  return instance;
}

ThemeService::ThemeService() : builtins_ready_(false) {
  EnsureBuiltinPacks();
  theme_ = MakeLightTheme();
  current_id_ = "light";
}

void ThemeService::EnsureBuiltinPacks() {
  if (builtins_ready_)
    return;
  builtins_ready_ = true;
  RegisterPack({"light", "Light", MakeLightTheme()});
  RegisterPack({"dark", "Dark", MakeDarkTheme()});
}

void ThemeService::RegisterPack(ThemePack pack) {
  if (pack.id.empty())
    return;
  for (ThemePack& existing : packs_) {
    if (existing.id == pack.id) {
      existing = std::move(pack);
      return;
    }
  }
  packs_.push_back(std::move(pack));
}

bool ThemeService::SetTheme(std::string_view id) {
  return SetTheme(id, true);
}

bool ThemeService::SetTheme(std::string_view id, bool persist_to_disk) {
  EnsureBuiltinPacks();
  const auto it =
      std::find_if(packs_.begin(), packs_.end(),
                   [id](const ThemePack& p) { return p.id == id; });
  if (it == packs_.end())
    return false;
  const bool id_changed = current_id_ != it->id;
  current_id_ = it->id;
  theme_ = it->colors;
  if (persist_to_disk && id_changed)
    Persist();
  if (id_changed) {
    for (ThemeObserver* obs : observers_) {
      if (obs)
        obs->OnThemeChanged();
    }
  }
  return true;
}

void ThemeService::AddObserver(ThemeObserver* observer) {
  if (!observer)
    return;
  if (std::find(observers_.begin(), observers_.end(), observer) ==
      observers_.end()) {
    observers_.push_back(observer);
  }
}

void ThemeService::RemoveObserver(ThemeObserver* observer) {
  observers_.erase(
      std::remove(observers_.begin(), observers_.end(), observer),
      observers_.end());
}

void ThemeService::LoadPersisted() {
  EnsureBuiltinPacks();
  const std::wstring path = ThemePersistFile();
  if (path.empty())
    return;
  FILE* fp = nullptr;
  if (_wfopen_s(&fp, path.c_str(), L"r") != 0 || !fp)
    return;
  char buf[64] = {};
  const size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
  fclose(fp);
  if (n == 0)
    return;
  std::string id(buf, n);
  while (!id.empty() &&
         (id.back() == '\n' || id.back() == '\r' || id.back() == ' ')) {
    id.pop_back();
  }
  if (!id.empty())
    SetTheme(id, false);
}

void ThemeService::Persist() const {
  const std::wstring dir = ThemePersistDir();
  const std::wstring path = ThemePersistFile();
  if (dir.empty() || path.empty() || current_id_.empty())
    return;
  CreateDirectoryW(dir.c_str(), nullptr);
  FILE* fp = nullptr;
  if (_wfopen_s(&fp, path.c_str(), L"w") != 0 || !fp)
    return;
  fwrite(current_id_.data(), 1, current_id_.size(), fp);
  fclose(fp);
}

}  // namespace views
