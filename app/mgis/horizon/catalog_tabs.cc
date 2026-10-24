// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/horizon/catalog_tabs.h"

#include "base/utf_string_conversions.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/controls/tree/tree_view.h"

namespace mgis {

CatalogTabs::CatalogTabs()
    : observer_(NULL),
      layer_model_(new NodeType(ASCIIToUTF16("Layers"), 0)),
      source_model_(new NodeType(ASCIIToUTF16("Sources"), 0)),
      map_model_(new NodeType(ASCIIToUTF16("Maps"), 0)),
      entity_model_(new NodeType(ASCIIToUTF16("Entities"), 0)),
      layer_tree_(NULL),
      source_tree_(NULL),
      map_tree_(NULL),
      entity_tree_(NULL),
      tabs_(NULL),
      suppress_observer_(false) {
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("Hillshade"), 1), 0);
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("Land"), 2), 1);
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("Lines"), 3), 2);
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("Points"), 4), 3);

  source_model_.GetRoot()->Add(
      new NodeType(ASCIIToUTF16("(no datasources)"), 1), 0);
  map_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("(no map docs)"), 1), 0);
  entity_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("tileset.dem"), 1), 0);
  entity_model_.GetRoot()->Add(
      new NodeType(ASCIIToUTF16("camera.view0"), 2), 1);

  layer_tree_ = new views::TreeView();
  layer_tree_->SetEditable(false);
  layer_tree_->SetRootShown(true);
  layer_tree_->SetModel(&layer_model_);

  source_tree_ = new views::TreeView();
  source_tree_->SetEditable(false);
  source_tree_->SetRootShown(true);
  source_tree_->SetModel(&source_model_);

  map_tree_ = new views::TreeView();
  map_tree_->SetEditable(false);
  map_tree_->SetRootShown(true);
  map_tree_->SetModel(&map_model_);

  // Not shown as a tab; retained for HasEntityTree() harness hooks.
  entity_tree_ = new views::TreeView();
  entity_tree_->SetEditable(false);
  entity_tree_->SetRootShown(true);
  entity_tree_->SetModel(&entity_model_);

  tabs_ = new views::TabbedPane();
  tabs_->set_listener(this);
  tabs_->AddTab(ASCIIToUTF16("Layers"),
                layer_tree_->CreateParentIfNecessary());
  tabs_->AddTab(ASCIIToUTF16("Sources"),
                source_tree_->CreateParentIfNecessary());
  tabs_->AddTab(ASCIIToUTF16("Maps"), map_tree_->CreateParentIfNecessary());
  AddChildView(tabs_);
}

CatalogTabs::~CatalogTabs() {
  if (layer_tree_)
    layer_tree_->SetModel(NULL);
  if (source_tree_)
    source_tree_->SetModel(NULL);
  if (map_tree_)
    map_tree_->SetModel(NULL);
  if (entity_tree_)
    entity_tree_->SetModel(NULL);
}

int CatalogTabs::selected_tab_index() const {
  return tabs_ ? tabs_->selected_tab_index() : -1;
}

void CatalogTabs::SelectTabAt(int index) {
  if (!tabs_)
    return;
  suppress_observer_ = true;
  tabs_->SelectTabAt(index);
  suppress_observer_ = false;
}

void CatalogTabs::Layout() {
  if (tabs_)
    tabs_->SetBounds(0, 0, width(), height());
}

void CatalogTabs::TabSelectedAt(int index) {
  if (suppress_observer_ || !observer_)
    return;
  observer_->OnCatalogTabSelected(index);
}

}  // namespace mgis
