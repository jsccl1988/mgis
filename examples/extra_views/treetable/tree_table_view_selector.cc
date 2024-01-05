// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tree_table_view_selector.h"

#include "base/i18n/case_conversion.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/base/range/range.h"
#include "tree_table_view.h"

namespace views {

namespace {

const int64 kTimeBeforeClearingMS = 1000;

}  // namespace

TreeTableViewSelector::TreeTableViewSelector(TreeTableView* tree)
    : tree_(tree) {
}

TreeTableViewSelector::~TreeTableViewSelector() {
}

void TreeTableViewSelector::OnTreeTableViewBlur() {
  ClearText();
}

void TreeTableViewSelector::SetCompositionText(
    const ui::CompositionText& composition) {
}

void TreeTableViewSelector::ConfirmCompositionText() {
}

void TreeTableViewSelector::ClearCompositionText() {
}

void TreeTableViewSelector::InsertText(const string16& text) {
  OnTextInput(text);
}

void TreeTableViewSelector::InsertChar(char16 ch, int flags) {
  OnTextInput(string16(1, ch));
}

ui::TextInputType TreeTableViewSelector::GetTextInputType() const {
  return ui::TEXT_INPUT_TYPE_TEXT;
}

bool TreeTableViewSelector::CanComposeInline() const {
  return false;
}

gfx::Rect TreeTableViewSelector::GetCaretBounds() {
  return gfx::Rect(tree_->GetVisibleBounds().origin(), gfx::Size());
}

bool TreeTableViewSelector::GetCompositionCharacterBounds(uint32 index,
                                                     gfx::Rect* rect) {
  *rect = GetCaretBounds();
  return false;
}

bool TreeTableViewSelector::HasCompositionText() {
  return false;
}

bool TreeTableViewSelector::GetTextRange(ui::Range* range) {
  *range = ui::Range();
  return false;
}

bool TreeTableViewSelector::GetCompositionTextRange(ui::Range* range) {
  *range = ui::Range();
  return false;
}

bool TreeTableViewSelector::GetSelectionRange(ui::Range* range) {
  *range = ui::Range();
  return false;
}

bool TreeTableViewSelector::SetSelectionRange(const ui::Range& range) {
  return false;
}

bool TreeTableViewSelector::DeleteRange(const ui::Range& range) {
  return false;
}

bool TreeTableViewSelector::GetTextFromRange(const ui::Range& range,
                                        string16* text) {
  return false;
}

void TreeTableViewSelector::OnInputMethodChanged() {
  ClearText();
}

bool TreeTableViewSelector::ChangeTextDirectionAndLayoutAlignment(
    base::i18n::TextDirection direction) {
  return true;
}

void TreeTableViewSelector::ExtendSelectionAndDelete(size_t before, size_t after) {
}

void TreeTableViewSelector::OnTextInput(const string16& text) {
  const int row_count = tree_->GetRowCount();
  if (row_count == 0)
    return;

  // Search for |text| if it has been a while since the user typed, otherwise
  // append |text| to |current_text_| and search for that. If it has been a
  // while search after the current row, otherwise search starting from the
  // current row.
  int row = (tree_->GetSelectedNode() != NULL) ?
      tree_->GetRowForNode(tree_->GetSelectedNode()) : 0;
  const base::TimeTicks now(base::TimeTicks::Now());
  if ((now - time_of_last_key_).InMilliseconds() < kTimeBeforeClearingMS) {
    current_text_ += text;
  } else {
    current_text_ = text;
    if (tree_->GetSelectedNode())
      row = (row + 1) % row_count;
  }
  time_of_last_key_ = now;

  const int start_row = row;
  const string16 lower_text(base::i18n::ToLower(current_text_));
  do {
    if (TextAtRowMatchesText(row, current_text_)) {
      tree_->SetSelectedNode(tree_->GetNodeForRow(row));
      return;
    }
    row = (row + 1) % row_count;
  } while (row != start_row);
}

bool TreeTableViewSelector::TextAtRowMatchesText(int row,
                                            const string16& lower_text) {
  ui::TreeTableModelNode* node = tree_->GetNodeForRow(row);
  DCHECK(node);
  const string16 model_text(base::i18n::ToLower(node->GetText(0)));
  return (model_text.size() >= lower_text.size()) &&
      (model_text.compare(0, lower_text.size(), lower_text) == 0);
}

void TreeTableViewSelector::ClearText() {
  current_text_.clear();
  time_of_last_key_ = base::TimeTicks();
}

}  // namespace views
