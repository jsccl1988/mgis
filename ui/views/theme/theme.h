// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_THEME_THEME_H_
#define UI_VIEWS_THEME_THEME_H_

#include <string_view>

#include "third_party/skia/include/core/SkColor.h"
#include "ui/views/views_export.h"

namespace views {

// Shell color snapshot for the active ThemeService pack.
// Field names mirror SmartGIS Theme so product chrome can share paint recipes.
struct VIEWS_EXPORT Theme {
  SkColor shell_bg = SkColorSetRGB(245, 245, 245);
  SkColor panel_bg = SkColorSetRGB(255, 255, 255);
  SkColor panel_header = SkColorSetRGB(232, 232, 232);
  SkColor accent = SkColorSetRGB(0, 120, 212);
  SkColor danger = SkColorSetRGB(196, 43, 28);
  SkColor text = SkColorSetRGB(50, 50, 50);
  SkColor text_bright = SkColorSetRGB(16, 16, 16);
  SkColor text_muted = SkColorSetRGB(110, 110, 110);
  SkColor control_bg = SkColorSetRGB(255, 255, 255);
  SkColor control_fill = SkColorSetRGB(225, 225, 225);
  SkColor control_hover = SkColorSetRGB(210, 210, 210);
  SkColor control_press = SkColorSetRGB(195, 195, 195);
  SkColor control_disabled = SkColorSetRGB(235, 235, 235);
  SkColor control_unchecked = SkColorSetRGB(220, 220, 220);
  // Idle edge for inputs / tables; focus uses accent, invalid uses danger.
  SkColor control_border = SkColorSetRGB(180, 180, 180);
  SkColor row_alt = SkColorSetRGB(248, 248, 248);
  SkColor map_placeholder = SkColorSetRGB(200, 220, 230);
  SkColor caption_bg = SkColorSetRGB(243, 243, 243);
  SkColor caption_button_hover = SkColorSetRGB(220, 220, 220);
  SkColor caption_close_hover = SkColorSetRGB(232, 17, 35);

  static const Theme& Current();

  // Resolves markup token names (e.g. "shell_bg", "text_muted").
  // Returns false when |name| is unknown.
  static bool ColorByName(std::string_view name, SkColor* out);
};

VIEWS_EXPORT Theme MakeDarkTheme();
VIEWS_EXPORT Theme MakeLightTheme();

}  // namespace views

#endif  // UI_VIEWS_THEME_THEME_H_
