// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/style/flex_style.h"

namespace views {
namespace {

template <typename T>
void MergeOpt(std::optional<T>* dst, const std::optional<T>& src) {
  if (src.has_value())
    *dst = src;
}

}  // namespace

void FlexStyle::MergeFrom(const FlexStyle& other) {
  MergeOpt(&display, other.display);
  MergeOpt(&flex_direction, other.flex_direction);
  MergeOpt(&justify_content, other.justify_content);
  MergeOpt(&align_items, other.align_items);
  MergeOpt(&flex, other.flex);
  MergeOpt(&flex_grow, other.flex_grow);
  MergeOpt(&flex_shrink, other.flex_shrink);
  MergeOpt(&gap, other.gap);
  MergeOpt(&padding, other.padding);
  MergeOpt(&padding_left, other.padding_left);
  MergeOpt(&padding_top, other.padding_top);
  MergeOpt(&padding_right, other.padding_right);
  MergeOpt(&padding_bottom, other.padding_bottom);
  MergeOpt(&margin, other.margin);
  MergeOpt(&margin_left, other.margin_left);
  MergeOpt(&margin_top, other.margin_top);
  MergeOpt(&margin_right, other.margin_right);
  MergeOpt(&margin_bottom, other.margin_bottom);
  MergeOpt(&width, other.width);
  MergeOpt(&height, other.height);
  MergeOpt(&min_width, other.min_width);
  MergeOpt(&min_height, other.min_height);
  MergeOpt(&max_width, other.max_width);
  MergeOpt(&max_height, other.max_height);
  MergeOpt(&color, other.color);
  MergeOpt(&background_color, other.background_color);
  MergeOpt(&font_size, other.font_size);
}

}  // namespace views
