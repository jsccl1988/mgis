// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_STYLE_FLEX_STYLE_H_
#define UI_VIEWS_MARKUP_STYLE_FLEX_STYLE_H_

#include <optional>

#include "third_party/skia/include/core/SkColor.h"
#include "ui/views/views_export.h"

namespace views {

// Flex CSS subset resolved onto one node (tag, then class, then id).
struct VIEWS_EXPORT FlexStyle {
  enum class Display {
    kFlex,
    kNone,
  };
  enum class FlexDirection {
    kRow,
    kColumn,
  };
  enum class Justify {
    kFlexStart,
    kFlexEnd,
    kCenter,
    kSpaceBetween,
    kSpaceAround,
    kSpaceEvenly,
  };
  enum class Align {
    kAuto,
    kFlexStart,
    kFlexEnd,
    kCenter,
    kStretch,
  };

  std::optional<Display> display;
  std::optional<FlexDirection> flex_direction;
  std::optional<Justify> justify_content;
  std::optional<Align> align_items;
  std::optional<float> flex;
  std::optional<float> flex_grow;
  std::optional<float> flex_shrink;
  std::optional<float> gap;
  std::optional<float> padding;
  std::optional<float> padding_left;
  std::optional<float> padding_top;
  std::optional<float> padding_right;
  std::optional<float> padding_bottom;
  std::optional<float> margin;
  std::optional<float> margin_left;
  std::optional<float> margin_top;
  std::optional<float> margin_right;
  std::optional<float> margin_bottom;
  std::optional<float> width;
  std::optional<float> height;
  std::optional<float> min_width;
  std::optional<float> min_height;
  std::optional<float> max_width;
  std::optional<float> max_height;
  std::optional<SkColor> color;
  std::optional<SkColor> background_color;
  std::optional<float> font_size;

  // Later rules win when the source optional is set.
  void MergeFrom(const FlexStyle& other);
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_STYLE_FLEX_STYLE_H_
