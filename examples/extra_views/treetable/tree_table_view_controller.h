// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_TREE_TREE_VIEW_CONTROLLER_H_
#define UI_VIEWS_CONTROLS_TREE_TREE_VIEW_CONTROLLER_H_

#include "ui/base/keycodes/keyboard_codes.h"
#include "../extra_views_export.h"

namespace ui {
class TreeTableModelNode;
}

namespace views {

class TreeTableView;

// TreeTableViewController ---------------------------------------------------------

// Controller for the treeview.
class EXTRA_EXPORT TreeTableViewController {
 public:
  // Notification that the selection of the tree view has changed. Use
  // GetSelectedNode to find the current selection.
  virtual void OnTreeTableViewSelectionChanged(TreeTableView* tree_view) = 0;

  // Returns true if the node can be edited. This is only used if the
  // TreeTableView is editable.
  virtual bool CanEdit(TreeTableView* tree_view, ui::TreeTableModelNode* node);

 protected:
  virtual ~TreeTableViewController();
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_TREE_TREE_VIEW_CONTROLLER_H_
