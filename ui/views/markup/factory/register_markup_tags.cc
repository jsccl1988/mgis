// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/register_markup_tags.h"

#include <memory>
#include <string>
#include <utility>

#include "ui/views/markup/factory/control_factory.h"
#include "ui/views/markup/factory/placeholder_view.h"
#include "ui/views/view.h"

namespace views {

void RegisterMarkupLayoutTags(ControlFactory* factory) {
  if (!factory)
    return;
  auto plain = [](std::string_view, const MarkupAttrs&) {
    return std::unique_ptr<View>(new View());
  };
  factory->RegisterTag("View", plain);
  factory->RegisterTag("FlexRow", plain);
  factory->RegisterTag("FlexColumn", plain);
  factory->RegisterTag("Placeholder",
                       [](std::string_view, const MarkupAttrs& attrs) {
                         const std::string text = attrs.Get("text");
                         const std::string id = attrs.Get("id");
                         std::string caption = !text.empty() ? text : id;
                         if (caption.empty())
                           caption = "placeholder";
                         return std::unique_ptr<View>(
                             new PlaceholderView(std::move(caption)));
                       });
}

}  // namespace views
