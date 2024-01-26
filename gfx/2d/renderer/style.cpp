
// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#include "gfx/2d/renderer/style.h"

#include "base/logging.h"

namespace gfx2d {
Style::Style(void) {
  name_ = "Default";
  style_type_ = ST_PenDesc;
}

Style::Style(const char *name, const PenDesc &pen_desc,
             const BrushDesc &brush_desc, const AnnotationDesc &anno_desc,
             const SymbolDesc &symbol_desc)
    : pen_desc_(pen_desc),
      brush_desc_(brush_desc),
      anno_desc_(anno_desc),
      symbol_desc_(symbol_desc) {
  SetStyleName(name);
}

Style::~Style() {}

Style *Style::Clone(const char *new_name) const {
  Style *pNewStyle =
      new Style(new_name, pen_desc_, brush_desc_, anno_desc_, symbol_desc_);
  if (pNewStyle != nullptr) {
    pNewStyle->SetStyleType(style_type_);
    return pNewStyle;
  } else
    return nullptr;
}

void StyleManager::SetDefaultStyle(const char *defName, PenDesc &pen_desc,
                                   BrushDesc &brush_desc,
                                   AnnotationDesc &anno_desc,
                                   SymbolDesc &symbol_desc) {
  default_style_ =
      new Style(defName, pen_desc, brush_desc, anno_desc, symbol_desc);
  style_list_.push_back(default_style_);
}

void StyleManager::DestroyAllStyle() {
  StyleList::iterator i = style_list_.begin();

  while (i != style_list_.end()) {
    SAFE_DELETE(*i);
    ++i;
  }
  style_list_.clear();
}

Style *StyleManager::CreateStyle(const char *defName, PenDesc &pen_desc,
                                 BrushDesc &brush_desc,
                                 AnnotationDesc &anno_desc,
                                 SymbolDesc &symbol_desc) {
  Style *style = GetStyle(defName);
  if (!style) {
    style = new Style(defName, pen_desc, brush_desc, anno_desc, symbol_desc);
    style_list_.push_back(style);
  }

  return style;
}

void StyleManager::DestroyStyle(const char *name) {
  Style *style = nullptr;
  StyleList::iterator i = style_list_.begin();

  while (i != style_list_.end()) {
    if (strcmp((**i).GetStyleName(), name) == 0) {
      SAFE_DELETE(*i);
      style_list_.erase(i);
      break;
    }
    ++i;
  }
}

void StyleManager::DestroyStyle(Style *style) {
  DestroyStyle(style->GetStyleName());
}

Style *StyleManager::GetStyle(const char *name) {
  Style *style = nullptr;
  StyleList::iterator i = style_list_.begin();

  while (i != style_list_.end()) {
    if (strcmp((**i).GetStyleName(), name) == 0) {
      style = *i;
      break;
    }
    ++i;
  }
  return style;
}

Style *StyleManager::GetDefaultStyle(void) {
  if (default_style_)
    return default_style_;
  else
    return nullptr;
}
}  // namespace gfx2d