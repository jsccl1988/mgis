// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_TREE_VIEW_EXAMPLE_H_
#define UI_VIEWS_EXAMPLES_TREE_VIEW_EXAMPLE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/views/context_menu_controller.h"
#include "ui/views/controls/button/button.h"
#include "examples/extra_views/treetable/tree_table_node_model.h"
#include "examples/extra_views/treetable/tree_table_view_controller.h"
#include "example_base.h"

namespace views {

class MenuRunner;
class TreeTableView;

namespace examples {

class TreeTableViewExample : public ExampleBase,
                        public ButtonListener,
                        public TreeTableViewController,
                        public ContextMenuController,
                        public ui::SimpleMenuModel::Delegate {
 public:
  TreeTableViewExample();
  virtual ~TreeTableViewExample();

  // ExampleBase:
  virtual void CreateExampleView(View* container) OVERRIDE;

 private:
  // IDs used by the context menu.
  enum MenuIDs {
    ID_EDIT,
    ID_REMOVE,
    ID_ADD
  };

  // Adds a new node.
  void AddNewNode();

  // Non-const version of IsCommandIdEnabled.
  bool IsCommandIdEnabled(int command_id);

  // ButtonListener:
  virtual void ButtonPressed(Button* sender, const ui::Event& event) OVERRIDE;

  // TreeTableViewController:
  virtual void OnTreeTableViewSelectionChanged(TreeTableView* tree_view) OVERRIDE;
  virtual bool CanEdit(TreeTableView* tree_view, ui::TreeTableModelNode* node) OVERRIDE;

  // ContextMenuController:
  virtual void ShowContextMenuForView(View* source,
                                      const gfx::Point& point) OVERRIDE;

  // SimpleMenuModel::Delegate:
  virtual bool IsCommandIdChecked(int command_id) const OVERRIDE;
  virtual bool IsCommandIdEnabled(int command_id) const OVERRIDE;
  virtual bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) OVERRIDE;
  virtual void ExecuteCommand(int command_id, int event_flags) OVERRIDE;

  // The tree view to be tested.
  TreeTableView* tree_view_;
  int column_for_edit;
  // Control buttons to modify the model.
  Button* add_;
  Button* remove_;
  Button* change_title_;

  typedef ui::TreeTableNodeWithValue<int> NodeType;

  ui::TreeTableNodeModel<NodeType> model_;

  scoped_ptr<MenuRunner> context_menu_runner_;

  DISALLOW_COPY_AND_ASSIGN(TreeTableViewExample);
};

}  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_TREE_VIEW_EXAMPLE_H_
