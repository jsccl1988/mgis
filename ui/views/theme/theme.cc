// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/theme/theme.h"

#include "ui/views/theme/theme_service.h"

namespace views {
namespace {

bool EqName(std::string_view a, const char* b) {
  return a == b;
}

}  // namespace

const Theme& Theme::Current() {
  return ThemeService::Get().theme();
}

bool Theme::ColorByName(std::string_view name, SkColor* out) {
  if (!out)
    return false;
  const Theme& t = Current();
  if (EqName(name, "shell_bg")) {
    *out = t.shell_bg;
    return true;
  }
  if (EqName(name, "panel_bg")) {
    *out = t.panel_bg;
    return true;
  }
  if (EqName(name, "panel_header")) {
    *out = t.panel_header;
    return true;
  }
  if (EqName(name, "accent")) {
    *out = t.accent;
    return true;
  }
  if (EqName(name, "danger")) {
    *out = t.danger;
    return true;
  }
  if (EqName(name, "text")) {
    *out = t.text;
    return true;
  }
  if (EqName(name, "text_bright")) {
    *out = t.text_bright;
    return true;
  }
  if (EqName(name, "text_muted")) {
    *out = t.text_muted;
    return true;
  }
  if (EqName(name, "control_bg")) {
    *out = t.control_bg;
    return true;
  }
  if (EqName(name, "control_fill")) {
    *out = t.control_fill;
    return true;
  }
  if (EqName(name, "control_hover")) {
    *out = t.control_hover;
    return true;
  }
  if (EqName(name, "control_press")) {
    *out = t.control_press;
    return true;
  }
  if (EqName(name, "control_disabled")) {
    *out = t.control_disabled;
    return true;
  }
  if (EqName(name, "control_unchecked")) {
    *out = t.control_unchecked;
    return true;
  }
  if (EqName(name, "control_border")) {
    *out = t.control_border;
    return true;
  }
  if (EqName(name, "row_alt")) {
    *out = t.row_alt;
    return true;
  }
  if (EqName(name, "map_placeholder")) {
    *out = t.map_placeholder;
    return true;
  }
  if (EqName(name, "caption_bg")) {
    *out = t.caption_bg;
    return true;
  }
  if (EqName(name, "caption_button_hover")) {
    *out = t.caption_button_hover;
    return true;
  }
  if (EqName(name, "caption_close_hover")) {
    *out = t.caption_close_hover;
    return true;
  }
  return false;
}

Theme MakeDarkTheme() {
  Theme t;
  t.shell_bg = SkColorSetRGB(30, 30, 30);
  t.panel_bg = SkColorSetRGB(37, 37, 38);
  t.panel_header = SkColorSetRGB(45, 45, 48);
  t.accent = SkColorSetRGB(0, 122, 204);
  t.danger = SkColorSetRGB(196, 43, 28);
  t.text = SkColorSetRGB(235, 235, 235);
  t.text_bright = SkColorSetRGB(255, 255, 255);
  t.text_muted = SkColorSetRGB(175, 175, 175);
  t.control_bg = SkColorSetRGB(30, 30, 30);
  t.control_fill = SkColorSetRGB(55, 55, 58);
  t.control_hover = SkColorSetRGB(78, 78, 82);
  t.control_press = SkColorSetRGB(48, 48, 52);
  t.control_disabled = SkColorSetRGB(45, 45, 45);
  t.control_unchecked = SkColorSetRGB(50, 50, 50);
  t.control_border = SkColorSetRGB(70, 70, 74);
  t.row_alt = SkColorSetRGB(34, 34, 36);
  t.map_placeholder = SkColorSetRGB(27, 58, 75);
  t.caption_bg = SkColorSetRGB(37, 37, 38);
  t.caption_button_hover = SkColorSetRGB(60, 60, 60);
  t.caption_close_hover = SkColorSetRGB(196, 43, 28);
  return t;
}

Theme MakeLightTheme() {
  Theme t;
  t.shell_bg = SkColorSetRGB(245, 245, 245);
  t.panel_bg = SkColorSetRGB(255, 255, 255);
  t.panel_header = SkColorSetRGB(232, 232, 232);
  t.accent = SkColorSetRGB(0, 120, 212);
  t.danger = SkColorSetRGB(196, 43, 28);
  t.text = SkColorSetRGB(50, 50, 50);
  t.text_bright = SkColorSetRGB(16, 16, 16);
  t.text_muted = SkColorSetRGB(110, 110, 110);
  t.control_bg = SkColorSetRGB(255, 255, 255);
  t.control_fill = SkColorSetRGB(225, 225, 225);
  t.control_hover = SkColorSetRGB(210, 210, 210);
  t.control_press = SkColorSetRGB(195, 195, 195);
  t.control_disabled = SkColorSetRGB(235, 235, 235);
  t.control_unchecked = SkColorSetRGB(220, 220, 220);
  t.control_border = SkColorSetRGB(180, 180, 180);
  t.row_alt = SkColorSetRGB(248, 248, 248);
  t.map_placeholder = SkColorSetRGB(200, 220, 230);
  t.caption_bg = SkColorSetRGB(243, 243, 243);
  t.caption_button_hover = SkColorSetRGB(220, 220, 220);
  t.caption_close_hover = SkColorSetRGB(232, 17, 35);
  return t;
}

}  // namespace views
