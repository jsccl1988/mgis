// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/control_factory.h"

#include "ui/views/markup/factory/register_markup_tags.h"

namespace views {

ControlFactory ControlFactory::MakeDefault() {
  ControlFactory factory;
  RegisterMarkupLayoutTags(&factory);
  RegisterChromiumControlTags(&factory);
  return factory;
}

const ControlFactory& ControlFactory::GetDefault() {
  static const ControlFactory kFactory = MakeDefault();
  return kFactory;
}

}  // namespace views
