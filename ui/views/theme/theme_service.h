// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_THEME_THEME_SERVICE_H_
#define UI_VIEWS_THEME_THEME_SERVICE_H_

#include <string>
#include <string_view>
#include <vector>

#include "ui/views/theme/theme.h"
#include "ui/views/views_export.h"

namespace views {

// Named theme pack: id is the stable wire/persist key; label is UI text.
struct VIEWS_EXPORT ThemePack {
  std::string id;
  std::string label;
  Theme colors;
};

// Observer notified after the active theme pack changes.
class VIEWS_EXPORT ThemeObserver {
 public:
  virtual ~ThemeObserver() {}
  virtual void OnThemeChanged() = 0;
};

// Process-wide theme pack registry (SmartGIS ThemeService shape, Chromium
// naming). Builtin packs: "light" (default) and "dark".
class VIEWS_EXPORT ThemeService {
 public:
  static ThemeService& Get();

  void EnsureBuiltinPacks();
  void RegisterPack(ThemePack pack);
  const std::vector<ThemePack>& packs() const { return packs_; }

  // |persist_to_disk| false keeps LocalAppData preference unchanged (tests).
  bool SetTheme(std::string_view id);
  bool SetTheme(std::string_view id, bool persist_to_disk);
  std::string_view current_id() const { return current_id_; }
  const Theme& theme() const { return theme_; }

  void AddObserver(ThemeObserver* observer);
  void RemoveObserver(ThemeObserver* observer);

  void LoadPersisted();
  void Persist() const;

 private:
  ThemeService();

  std::vector<ThemePack> packs_;
  std::string current_id_;
  Theme theme_;
  std::vector<ThemeObserver*> observers_;
  bool builtins_ready_;
};

}  // namespace views

#endif  // UI_VIEWS_THEME_THEME_SERVICE_H_
