// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_FACTORY_CONTROL_FACTORY_H_
#define UI_VIEWS_MARKUP_FACTORY_CONTROL_FACTORY_H_

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ui/views/views_export.h"

namespace views {

class View;

// Attribute bag for XML element construction. Keeps pugixml out of this header.
struct VIEWS_EXPORT MarkupAttrs {
  MarkupAttrs();
  MarkupAttrs(const MarkupAttrs&);
  MarkupAttrs(MarkupAttrs&&) noexcept;
  MarkupAttrs& operator=(const MarkupAttrs&);
  MarkupAttrs& operator=(MarkupAttrs&&) noexcept;
  ~MarkupAttrs();

  std::string Get(std::string_view key, std::string_view fallback = {}) const;
  std::unordered_map<std::string, std::string> values;
};

// Maps XML tag names to View constructors. Tags are matched case-insensitively.
class VIEWS_EXPORT ControlFactory {
 public:
  using Creator = std::function<std::unique_ptr<View>(std::string_view tag,
                                                      const MarkupAttrs& attrs)>;

  ControlFactory();

  void RegisterTag(std::string_view tag, Creator creator);
  bool HasTag(std::string_view tag) const;

  // Returns null for unknown tags.
  std::unique_ptr<View> Create(std::string_view tag,
                               const MarkupAttrs& attrs) const;

  std::vector<std::string> RegisteredTags() const;

  // Layout sugar plus Chromium control tags.
  static ControlFactory MakeDefault();
  // Process-wide default registry.
  static const ControlFactory& GetDefault();

 private:
  std::unordered_map<std::string, Creator> creators_;
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_FACTORY_CONTROL_FACTORY_H_
