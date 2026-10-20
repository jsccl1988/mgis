// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_DOCUMENT_NAMED_VIEW_MAP_H_
#define UI_VIEWS_MARKUP_DOCUMENT_NAMED_VIEW_MAP_H_

#include <string>
#include <string_view>
#include <unordered_map>

#include "ui/views/views_export.h"

namespace views {

class View;

// Id to View* table filled while loading markup. Does not own the Views.
class VIEWS_EXPORT NamedViewMap {
 public:
  void Clear();
  void Put(std::string_view id, View* view);
  View* Find(std::string_view id) const;
  bool Contains(std::string_view id) const;

  template <typename T>
  T* FindAs(std::string_view id) const {
    return static_cast<T*>(Find(id));
  }

  const std::unordered_map<std::string, View*>& entries() const {
    return map_;
  }

 private:
  std::unordered_map<std::string, View*> map_;
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_DOCUMENT_NAMED_VIEW_MAP_H_
