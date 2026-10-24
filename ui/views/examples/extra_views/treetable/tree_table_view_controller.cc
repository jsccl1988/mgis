// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tree_table_view_controller.h"

#include "tree_table_model.h"
#include "tree_table_view.h"

namespace views {

bool TreeTableViewController::CanEdit(TreeTableView* tree_view, ui::TreeTableModelNode* node) {
  return true;
}

TreeTableViewController::~TreeTableViewController() {
}

}  // namespace views
