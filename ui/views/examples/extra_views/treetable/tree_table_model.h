// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_MODELS_TREE_TABLE_MODEL_H_
#define UI_BASE_MODELS_TREE_TABLE_MODEL_H_

#include <vector>

#include "base/string16.h"
#include "../extra_views_export.h"

namespace gfx {
class ImageSkia;
}

namespace ui {

class TreeTableModel;

// TreeTableModelNode --------------------------------------------------------------

// Type of class returned from the model.
class EXTRA_EXPORT TreeTableModelNode {
 public:
  // Returns the title for the node.
  virtual const base::string16& GetText(int column_index) const = 0;
  virtual void SetText(int column_index, const base::string16& text) = 0;
 protected:
	 virtual ~TreeTableModelNode() {}
};

// Observer for the // TreeTableModel ------------------------------------------------------------------. Notified of significant events to the model.
class EXTRA_EXPORT TreeTableModelObserver {
 public:
  // Notification that nodes were added to the specified parent.
  virtual void TreeTableNodesAdded(TreeTableModel* model,
                              TreeTableModelNode* parent,
                              int start,
                              int count) = 0;

  // Notification that nodes were removed from the specified parent.
  virtual void TreeTableNodesRemoved(TreeTableModel* model,
                                TreeTableModelNode* parent,
                                int start,
                                int count) = 0;

  // Notification that the contents of a node has changed.
  virtual void TreeTableNodeChanged(TreeTableModel* model, TreeTableModelNode* node) = 0;

 protected:
  virtual ~TreeTableModelObserver() {}
};

// TreeTableModel ------------------------------------------------------------------

// The model for TreeTableView.
class EXTRA_EXPORT TreeTableModel {
 public:
  // Returns the root of the tree. This may or may not be shown in the tree,
  // see SetRootShown for details.
  virtual TreeTableModelNode* GetRoot() = 0;

  // Returns the number of children in |parent|.
  virtual int GetChildCount(TreeTableModelNode* parent) = 0;

  // Returns the child node of |parent| at |index|.
  virtual TreeTableModelNode* GetChild(TreeTableModelNode* parent, int index) = 0;

  // Returns the index of |child| in |parent|.
  virtual int GetIndexOf(TreeTableModelNode* parent, TreeTableModelNode* child) = 0;

  // Returns the parent of |node|, or NULL if |node| is the root.
  virtual TreeTableModelNode* GetParent(TreeTableModelNode* node) = 0;

  // Adds an observer of the model.
  virtual void AddObserver(TreeTableModelObserver* observer) = 0;

  // Removes an observer of the model.
  virtual void RemoveObserver(TreeTableModelObserver* observer) = 0;

  // Sets the title of |node|.
  // This is only invoked if the node is editable and the user edits a node.
  virtual void SetText(TreeTableModelNode* node, int column_index, const base::string16& text) = 0;

  virtual const base::string16& GetText(TreeTableModelNode* node, int column_index) const = 0;

  // Returns the set of icons for the nodes in the tree. You only need override
  // this if you don't want to use the default folder icons.
  virtual void GetIcons(std::vector<gfx::ImageSkia>* icons) {}

  // Returns the index of the icon to use for |node|. Return -1 to use the
  // default icon. The index is relative to the list of icons returned from
  // GetIcons.
  virtual int GetIconIndex(TreeTableModelNode* node);

 protected:
	virtual ~TreeTableModel() {}
};

struct EXTRA_EXPORT TreeTableColumn {
	enum Alignment {
		LEFT, RIGHT, CENTER
	};

	TreeTableColumn();
	TreeTableColumn(int id, Alignment alignment, int width, float percent);

	// A unique identifier for the column.
	int id;

	// The title for the column.
	base::string16 title;

	// Alignment for the content.
	Alignment alignment;

	// The size of a column may be specified in two ways:
	// 1. A fixed width. Set the width field to a positive number and the
	//    column will be given that width, in pixels.
	// 2. As a percentage of the available width. If width is -1, and percent is
	//    > 0, the column is given a width of
	//    available_width * percent / total_percent.
	// 3. If the width == -1 and percent == 0, the column is autosized based on
	//    the width of the column header text.
	//
	// Sizing is done in four passes. Fixed width columns are given
	// their width, percentages are applied, autosized columns are autosized,
	// and finally percentages are applied again taking into account the widths
	// of autosized columns.
	int width;
	float percent;

	// The minimum width required for all items in this column
	// (including the header)
	// to be visible.
	int min_visible_width;

	// Is this column sortable? Default is false
	bool sortable;
};

}  // namespace ui

#endif  // UI_BASE_MODELS_TREE_TABLE_MODEL_H_
