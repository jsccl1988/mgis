// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_HORIZON_CATALOG_TABS_H_
#define APP_MGIS_HORIZON_CATALOG_TABS_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/models/tree_node_model.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "ui/views/view.h"

namespace views {
class TabbedPane;
class TreeView;
}

namespace mgis {

// Left catalog: Layers / Sources / Maps (SmartGIS chrome). Catalog tabs are
// independent of the center Map|3D scene mode.
class CatalogTabs : public views::View, public views::TabbedPaneListener {
 public:
  typedef ui::TreeNodeWithValue<int> NodeType;

  class Observer {
   public:
    virtual void OnCatalogTabSelected(int index) = 0;

   protected:
    virtual ~Observer() {}
  };

  CatalogTabs();
  virtual ~CatalogTabs();

  void set_observer(Observer* observer) { observer_ = observer; }

  ui::TreeNodeModel<NodeType>* layer_model() { return &layer_model_; }
  ui::TreeNodeModel<NodeType>* source_model() { return &source_model_; }
  ui::TreeNodeModel<NodeType>* map_model() { return &map_model_; }
  // Kept for harness callers that still ask for an entity tree pointer.
  ui::TreeNodeModel<NodeType>* entity_model() { return &entity_model_; }

  views::TreeView* layer_tree() const { return layer_tree_; }
  views::TreeView* source_tree() const { return source_tree_; }
  views::TreeView* map_tree() const { return map_tree_; }
  views::TreeView* entity_tree() const { return entity_tree_; }

  int selected_tab_index() const;
  void SelectTabAt(int index);

  virtual void Layout() OVERRIDE;
  virtual void TabSelectedAt(int index) OVERRIDE;

 private:
  Observer* observer_;
  ui::TreeNodeModel<NodeType> layer_model_;
  ui::TreeNodeModel<NodeType> source_model_;
  ui::TreeNodeModel<NodeType> map_model_;
  ui::TreeNodeModel<NodeType> entity_model_;
  views::TreeView* layer_tree_;
  views::TreeView* source_tree_;
  views::TreeView* map_tree_;
  views::TreeView* entity_tree_;
  views::TabbedPane* tabs_;
  bool suppress_observer_;

  DISALLOW_COPY_AND_ASSIGN(CatalogTabs);
};

}  // namespace mgis

#endif  // APP_MGIS_HORIZON_CATALOG_TABS_H_
