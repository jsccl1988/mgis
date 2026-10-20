// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/document/named_view_map.h"

namespace views {

void NamedViewMap::Clear() {
  map_.clear();
}

void NamedViewMap::Put(std::string_view id, View* view) {
  if (id.empty() || !view)
    return;
  map_[std::string(id)] = view;
}

View* NamedViewMap::Find(std::string_view id) const {
  const auto it = map_.find(std::string(id));
  if (it == map_.end())
    return NULL;
  return it->second;
}

bool NamedViewMap::Contains(std::string_view id) const {
  return map_.find(std::string(id)) != map_.end();
}

}  // namespace views
