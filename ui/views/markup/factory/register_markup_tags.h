// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_FACTORY_REGISTER_MARKUP_TAGS_H_
#define UI_VIEWS_MARKUP_FACTORY_REGISTER_MARKUP_TAGS_H_

#include "ui/views/views_export.h"

namespace views {

class ControlFactory;

// View / FlexRow / FlexColumn / Placeholder.
VIEWS_EXPORT void RegisterMarkupLayoutTags(ControlFactory* factory);

// Chromium controls: Label, LabelButton, Textfield, Checkbox, RadioButton,
// Combobox, Slider, ScrollView, ImageView, Separator.
VIEWS_EXPORT void RegisterChromiumControlTags(ControlFactory* factory);

}  // namespace views

#endif  // UI_VIEWS_MARKUP_FACTORY_REGISTER_MARKUP_TAGS_H_
