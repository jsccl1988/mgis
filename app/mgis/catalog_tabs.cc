// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/catalog_tabs.h"

#include "base/utf_string_conversions.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/controls/tree/tree_view.h"

namespace mgis {

CatalogTabs::CatalogTabs()
    : observer_(NULL),
      layer_model_(new NodeType(WideToUTF16(L"Layers"), 0)),
      entity_model_(new NodeType(WideToUTF16(L"Entities"), 0)),
      layer_tree_(NULL),
      entity_tree_(NULL),
      tabs_(NULL),
      suppress_observer_(false) {
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("basemap"), 1), 0);
  layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("parcels"), 2), 1);
  entity_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("tileset.dem"), 1), 0);
  entity_model_.GetRoot()->Add(
      new NodeType(ASCIIToUTF16("camera.view0"), 2), 1);

  layer_tree_ = new views::TreeView();
  layer_tree_->SetEditable(false);
  layer_tree_->SetRootShown(true);
  layer_tree_->SetModel(&layer_model_);

  entity_tree_ = new views::TreeView();
  entity_tree_->SetEditable(false);
  entity_tree_->SetRootShown(true);
  entity_tree_->SetModel(&entity_model_);

  tabs_ = new views::TabbedPane();
  tabs_->set_listener(this);
  tabs_->AddTab(WideToUTF16(L"\u56fe\u5c42"),
                layer_tree_->CreateParentIfNecessary());
  tabs_->AddTab(WideToUTF16(L"\u5b9e\u4f53"),
                entity_tree_->CreateParentIfNecessary());
  AddChildView(tabs_);
}

CatalogTabs::~CatalogTabs() {
  if (layer_tree_)
    layer_tree_->SetModel(NULL);
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
