// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_TREE_TABLE_VIEW_VIEWS_H_
#define UI_VIEWS_CONTROLS_TREE_TABLE_VIEW_VIEWS_H_

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "tree_table_node_model.h"
#include "ui/gfx/font.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/view.h"

namespace views {

class Textfield;
class TreeTableViewController;
class TreeTableViewSelector;
class TreeTableHeader;
// TreeView displays hierarchical data as returned from a TreeTableModel. The user
// can expand, collapse and edit the items. A Controller may be attached to
// receive notification of selection changes and restrict editing.
//
// Note on implementation. This implementation doesn't scale well. In particular
// it does not store any row information, but instead calculates it as
// necessary. But it's more than adequate for current uses.
class EXTRA_EXPORT TreeTableView : public View,
                              public ui::TreeTableModelObserver,
                              public TextfieldController,
                              public FocusChangeListener {
 public:
	 static std::string kViewClassName;
	 struct EXTRA_EXPORT VisibleColumn {
		 VisibleColumn();
		 ~VisibleColumn();

		 // The column.
		 ui::TreeTableColumn column;

		 // Starting x-coordinate of the column.
		 int x;

	  // Starting x-coordinate of the column.
		 int width;
	 };

 
  struct  SortDescriptor {
	  SortDescriptor() : column_id(-1), ascending(true) {}
	  SortDescriptor(int column_id, bool ascending)
		  : column_id(column_id),
		  ascending(ascending) {}

	  // ID of the sorted column.
	  int column_id;

	  // Is the sort ascending?
	  bool ascending;
  };
  typedef std::vector<SortDescriptor> SortDescriptors;
 TreeTableView();
 TreeTableView(const std::vector<ui::TreeTableColumn>& columns);
  virtual ~TreeTableView();

  // Returns new ScrollPane that contains the receiver.
  View* CreateParentIfNecessary();

  void CreateHeaderIfNecessary();
  // Sets the model. TreeTableView does not take ownership of the model.
  void SetModel(ui::TreeTableModel* model);
  ui::TreeTableModel* model() const { return model_; }

  std::vector<ui::TreeTableColumn> columns_;
  // Sets whether to automatically expand children when a parent node is
  // expanded. The default is false. If true, when a node in the tree is
  // expanded for the first time, its children are also automatically expanded.
  // If a node is subsequently collapsed and expanded again, the children
  // will not be automatically expanded.
  void set_auto_expand_children(bool auto_expand_children) {
    auto_expand_children_ = auto_expand_children;
  }

  // Sets whether the user can edit the nodes. The default is true. If true,
  // the Controller is queried to determine if a particular node can be edited.
  void SetEditable(bool editable);

  // Edits the specified node. This cancels the current edit and expands all
  // parents of node.
  void StartEditing(ui::TreeTableModelNode* node, int column = 0);

  // Cancels the current edit. Does nothing if not editing.
  void CancelEdit();

  // Commits the current edit. Does nothing if not editing.
  void CommitEdit();

  // If the user is editing a node, it is returned. If the user is not
  // editing a node, NULL is returned.
  ui::TreeTableModelNode* GetEditingNode();

  // Selects the specified node. This expands all the parents of node.
  void SetSelectedNode(ui::TreeTableModelNode* model_node);

  // Returns the selected node, or NULL if nothing is selected.
  ui::TreeTableModelNode* GetSelectedNode();

  // Marks |model_node| as collapsed. This only effects the UI if node and all
  // its parents are expanded (IsExpanded(model_node) returns true).
  void Collapse(ui::TreeTableModelNode* model_node);

  // Make sure node and all its parents are expanded.
  void Expand(ui::TreeTableModelNode* node);

  // Invoked from ExpandAll(). Expands the supplied node and recursively
  // invokes itself with all children.
  void ExpandAll(ui::TreeTableModelNode* node);

  // Returns true if the specified node is expanded.
  bool IsExpanded(ui::TreeTableModelNode* model_node);

  // Sets whether the root is shown. If true, the root node of the tree is
  // shown, if false only the children of the root are shown. The default is
  // true.
  void SetRootShown(bool root_visible);

  // Sets the controller, which may be null. TreeTableView does not take ownership
  // of the controller.
  void SetController(TreeTableViewController* controller) {
    controller_ = controller;
  }

  // Returns the number of rows.
  int GetRowCount();

  // Returns the node for the specified row, or NULL for an invalid row index.
  ui::TreeTableModelNode* GetNodeForRow(int row);

  // Maps a node to a row, returns -1 if node is not valid.
  int GetRowForNode(ui::TreeTableModelNode* node);

  views::Textfield* editor() { return editor_; }

  // View overrides:
  virtual void Layout() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual bool AcceleratorPressed(const ui::Accelerator& accelerator) OVERRIDE;
  virtual bool OnMousePressed(const ui::MouseEvent& event) OVERRIDE;
  virtual ui::TextInputClient* GetTextInputClient() OVERRIDE;
  virtual void OnGestureEvent(ui::GestureEvent* event) OVERRIDE;
  virtual void ShowContextMenu(const gfx::Point& p,bool is_mouse_gesture) OVERRIDE;
  virtual void GetAccessibleState(ui::AccessibleViewState* state) OVERRIDE;
  virtual std::string GetClassName() const;

  // TreeTableModelObserver overrides:
  virtual void TreeTableNodesAdded(ui::TreeTableModel* model,
                              ui::TreeTableModelNode* parent,
                              int start,
                              int count) OVERRIDE;
  virtual void TreeTableNodesRemoved(ui::TreeTableModel* model,
                                ui::TreeTableModelNode* parent,
                                int start,
                                int count) OVERRIDE;
  virtual void TreeTableNodeChanged(ui::TreeTableModel* model,
                               ui::TreeTableModelNode* model_node) OVERRIDE;

  // TextfieldController overrides:
  virtual void ContentsChanged(Textfield* sender,
                               const string16& new_contents) OVERRIDE;
  virtual bool HandleKeyEvent(Textfield* sender,
                              const ui::KeyEvent& key_event) OVERRIDE;

  // FocusChangeListener overrides:
  virtual void OnWillChangeFocus(View* focused_before,
                                 View* focused_now) OVERRIDE;
  virtual void OnDidChangeFocus(View* focused_before,
                                View* focused_now) OVERRIDE;

  int GetNodeColumnIndexFromPoint(ui::TreeTableModelNode* node, gfx::Point point);

  int  GetColumnWidth(int column_index) const;
  void UpdateVisibleColumnSizes();
  void SetVisibleColumnWidth(int index, int width);
  //Column header
  const std::vector<VisibleColumn>& visible_columns() const {
	  return visible_columns_;
  }
  
  void ToggleSortOrder(int visible_column_index);
  bool in_set_visible_column_width_;
  std::vector<VisibleColumn> visible_columns_;
  SortDescriptors sort_descriptors_;
  const SortDescriptors& sort_descriptors() const { return sort_descriptors_; }
  void SetSortDescriptors(const SortDescriptors& sort_descriptors);


  TreeTableHeader* header_;
 protected:
  // View overrides:
  virtual gfx::Point GetKeyboardContextMenuLocation() OVERRIDE;
  virtual bool OnKeyPressed(const ui::KeyEvent& event) OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void OnFocus() OVERRIDE;
  virtual void OnBlur() OVERRIDE;

 private:

  // Selects, expands or collapses nodes in the tree.  Consistent behavior for
  // tap gesture and click events.
  bool OnClickOrTap(const ui::LocatedEvent& event);

  // InternalNode is used to track information about the set of nodes displayed
  // by TreeTableViewViews.
  class InternalNode : public ui::TreeTableNode<InternalNode> {
   public:
    InternalNode();
    virtual ~InternalNode();

    // Resets the state from |node|.
    void Reset(ui::TreeTableModelNode* node);

    // The model node this InternalNode represents.
    ui::TreeTableModelNode* model_node() { return model_node_; }

    // Whether the node is expanded.
    void set_is_expanded(bool expanded) { is_expanded_ = expanded; }
    bool is_expanded() const { return is_expanded_; }

    // Whether children have been loaded.
    void set_loaded_children(bool value) { loaded_children_ = value; }
    bool loaded_children() const { return loaded_children_; }

    // Width needed to display the string.
    void set_text_width(int width) { text_width_ = width; }
    int text_width() const { return text_width_; }

    // Returns the total number of descendants (including this node).
    int NumExpandedNodes();

    // Returns the max width of all descendants (including this node). |indent|
    // is how many pixels each child is indented and |depth| is the depth of
    // this node from its parent.
    int GetMaxWidth(int indent, int depth);

	virtual void SetText(int column_index, const base::string16& text) {
		model_node_->SetText(column_index, text);
	}

	// TreeModelNode:
	virtual const base::string16& GetText(int column_index) const{
		return model_node_->GetText(column_index);
	}

   private:
    // The node from the model.
    ui::TreeTableModelNode* model_node_;

    // Whether the children have been loaded.
    bool loaded_children_;

    bool is_expanded_;

    int text_width_;

    DISALLOW_COPY_AND_ASSIGN(InternalNode);
  };

  // Used by GetInternalNodeForModelNode.
  enum GetInternalNodeCreateType {
    // If an InternalNode hasn't been created yet, create it.
    CREATE_IF_NOT_LOADED,

    // Don't create an InternalNode if one hasn't been created yet.
    DONT_CREATE_IF_NOT_LOADED,
  };

  // Used by IncrementSelection.
  enum IncrementType {
    // Selects the next node.
    INCREMENT_NEXT,

    // Selects the previous node.
    INCREMENT_PREVIOUS
  };

  // Row of the root node. This varies depending upon whether the root is
  // visible.
  int root_row() const { return root_shown_ ? 0 : -1; }

  // Depth of the root node.
  int root_depth() const { return root_shown_ ? 0 : -1; }

  // Loads the children of the specified node.
  void LoadChildren(InternalNode* node);

  // Configures an InternalNode from a node from the model. This is used
  // when a node changes as well as when loading.
  void ConfigureInternalNode(ui::TreeTableModelNode* model_node, InternalNode* node);

  // Sets |node|s text_width.
  void UpdateNodeTextWidth(InternalNode* node);

  // Invoked when the set of drawn nodes changes.
  void DrawnNodesChanged();

  // Updates |preferred_size_| from the state of the UI.
  void UpdatePreferredSize();

  // Positions |editor_|.
  void LayoutEditor(int column = 0);

  // Schedules a paint for |node|.
  void SchedulePaintForNode(InternalNode* node);

  // Recursively paints rows from |min_row| to |max_row|. |node| is the node for
  // the row |*row|. |row| is updated as this walks the tree. Depth is the depth
  // of |*row|.
  void PaintRows(gfx::Canvas* canvas,
                 int min_row,
                 int max_row,
                 InternalNode* node,
                 int depth,
                 int* row);

  // Invoked to paint a single node.
  void PaintRow(gfx::Canvas* canvas,
                InternalNode* node,
                int row,
                int depth);

  // Paints the expand control given the specified nodes bounds.
  void PaintExpandControl(gfx::Canvas* canvas,
                          const gfx::Rect& node_bounds,
                          bool expanded);

  // Returns the InternalNode for a model node. |create_type| indicates wheter
  // this should load InternalNode or not.
  InternalNode* GetInternalNodeForModelNode(
      ui::TreeTableModelNode* model_node,
      GetInternalNodeCreateType create_type);

  // Returns the bounds for a node.
  gfx::Rect GetBoundsForNode(InternalNode* node);

  gfx::Rect GetCellBoundsForNode(InternalNode* node, size_t column);

  

  // Implementation of GetBoundsForNode. Separated out as some callers already
  // know the row/depth.
  gfx::Rect GetBoundsForNodeImpl(InternalNode* node, int row, int depth);

  // Returns the row and depth of a node.
  int GetRowForInternalNode(InternalNode* node, int* depth);

  // Returns the row and depth of the specified node.
  InternalNode* GetNodeByRow(int row, int* depth);

  // Implementation of GetNodeByRow. |curent_row| is updated as we iterate.
  InternalNode* GetNodeByRowImpl(InternalNode* node,
                                 int target_row,
                                 int current_depth,
                                 int* current_row,
                                 int* node_depth);

  // Increments the selection. Invoked in response to up/down arrow.
  void IncrementSelection(IncrementType type);

  // If the current node is expanded, it's collapsed, otherwise selection is
  // moved to the parent.
  void CollapseOrSelectParent();

  // If the selected node is collapsed, it's expanded. Otherwise the first child
  // is seleected.
  void ExpandOrSelectChild();

  // Implementation of Expand(). Returns true if at least one node was expanded
  // that previously wasn't.
  bool ExpandImpl(ui::TreeTableModelNode* model_node);

  // The model, may be null.
  ui::TreeTableModel* model_;

  // Default icons for closed/open.
  gfx::ImageSkia closed_icon_;
  gfx::ImageSkia open_icon_;

  // Icons from the model.
  std::vector<gfx::ImageSkia> icons_;

  // The root node.
  InternalNode root_;

  // The selected node, may be NULL.
  InternalNode* selected_node_;

  bool editing_;

  // The editor; lazily created and never destroyed (well, until TreeTableView is
  // destroyed). Hidden when no longer editing. We do this avoid destruction
  // problems.
  Textfield* editor_;

  // Preferred size of |editor_| with no content.
  gfx::Size empty_editor_size_;

  // If non-NULL we've attached a listener to this focus manager. Used to know
  // when focus is changing to another view so that we can cancel the edit.
  FocusManager* focus_manager_;

  // Whether to automatically expand children when a parent node is expanded.
  bool auto_expand_children_;

  // Whether the user can edit the items.
  bool editable_;

  // The controller.
  TreeTableViewController* controller_;

  // Whether or not the root is shown in the tree.
  bool root_shown_;

  // Did the model return a non-empty set of icons from GetIcons?
  bool has_custom_icons_;

  // Cached preferred size.
  gfx::Size preferred_size_;

  // Font used to display text.
  gfx::Font font_;

  // Height of each row. Based on font and some padding.
  int row_height_;

  // Offset the text is drawn at. This accounts for the size of the expand
  // control, icon and offsets.
  int text_offset_;

  int last_parent_width_;

  scoped_ptr<TreeTableViewSelector> selector_;

  DISALLOW_COPY_AND_ASSIGN(TreeTableView);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_TREE_TABLE_VIEW_VIEWS_H_
