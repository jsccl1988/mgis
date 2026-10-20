// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/register_markup_tags.h"

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/radio_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/separator.h"
#include "ui/views/controls/slider.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/markup/factory/control_factory.h"
#include "ui/views/markup/factory/markup_combobox.h"

namespace views {
namespace {

string16 AttrText(const MarkupAttrs& attrs) {
  return UTF8ToUTF16(attrs.Get("text"));
}

std::string ToLower(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    out.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return out;
}

}  // namespace

void RegisterChromiumControlTags(ControlFactory* factory) {
  if (!factory)
    return;

  factory->RegisterTag("Label", [](std::string_view, const MarkupAttrs& attrs) {
    return std::unique_ptr<View>(new Label(AttrText(attrs)));
  });

  factory->RegisterTag(
      "LabelButton", [](std::string_view, const MarkupAttrs& attrs) {
        return std::unique_ptr<View>(new LabelButton(NULL, AttrText(attrs)));
      });

  factory->RegisterTag(
      "Textfield", [](std::string_view, const MarkupAttrs& attrs) {
        std::unique_ptr<Textfield> field(new Textfield());
        field->SetText(AttrText(attrs));
        return std::unique_ptr<View>(field.release());
      });

  factory->RegisterTag(
      "Checkbox", [](std::string_view, const MarkupAttrs& attrs) {
        return std::unique_ptr<View>(new Checkbox(AttrText(attrs)));
      });

  factory->RegisterTag(
      "RadioButton", [](std::string_view, const MarkupAttrs& attrs) {
        const std::string group = attrs.Get("group", "0");
        const int group_id = std::atoi(group.c_str());
        return std::unique_ptr<View>(
            new RadioButton(AttrText(attrs), group_id));
      });

  factory->RegisterTag(
      "Combobox", [](std::string_view, const MarkupAttrs&) {
        return std::unique_ptr<View>(new MarkupCombobox());
      });

  factory->RegisterTag("Slider", [](std::string_view, const MarkupAttrs& attrs) {
    const std::string orientation = ToLower(attrs.Get("orientation", "horizontal"));
    const Slider::Orientation ori =
        (orientation == "vertical" || orientation == "v") ? Slider::VERTICAL
                                                          : Slider::HORIZONTAL;
    std::unique_ptr<Slider> slider(new Slider(NULL, ori));
    const std::string value = attrs.Get("value");
    if (!value.empty())
      slider->SetValue(static_cast<float>(std::atof(value.c_str())));
    return std::unique_ptr<View>(slider.release());
  });

  factory->RegisterTag("ScrollView", [](std::string_view, const MarkupAttrs&) {
    return std::unique_ptr<View>(new ScrollView());
  });

  factory->RegisterTag("ImageView", [](std::string_view, const MarkupAttrs&) {
    return std::unique_ptr<View>(new ImageView());
  });

  factory->RegisterTag("Separator", [](std::string_view, const MarkupAttrs&) {
    return std::unique_ptr<View>(new Separator());
  });
}

}  // namespace views
