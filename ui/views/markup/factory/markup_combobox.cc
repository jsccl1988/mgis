// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/markup_combobox.h"

#include "ui/views/controls/combobox/combobox.h"

namespace views {

int MarkupCombobox::Model::GetItemCount() const {
  return static_cast<int>(items_.size());
}

string16 MarkupCombobox::Model::GetItemAt(int index) {
  if (index < 0 || index >= GetItemCount())
    return string16();
  return items_[static_cast<size_t>(index)];
}

void MarkupCombobox::Model::Add(const string16& text) {
  items_.push_back(text);
}

MarkupCombobox::MarkupCombobox() : combo_(NULL) {
  combo_ = new Combobox(&model_);
  AddChildView(combo_);
}

MarkupCombobox::~MarkupCombobox() {}

void MarkupCombobox::AddItem(const string16& text) {
  model_.Add(text);
  if (combo_)
    combo_->ModelChanged();
}

int MarkupCombobox::item_count() const {
  return model_.GetItemCount();
}

void MarkupCombobox::Layout() {
  if (combo_)
    combo_->SetBounds(0, 0, width(), height());
}

gfx::Size MarkupCombobox::GetPreferredSize() {
  if (combo_)
    return combo_->GetPreferredSize();
  return gfx::Size(120, 24);
}

std::string MarkupCombobox::GetClassName() const {
  return "views/MarkupCombobox";
}

}  // namespace views
