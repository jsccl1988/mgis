// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_STYLE_CSS_PARSER_H_
#define UI_VIEWS_MARKUP_STYLE_CSS_PARSER_H_

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ui/views/markup/style/flex_style.h"
#include "ui/views/views_export.h"

namespace views {

// Parses the v1 Flex CSS subset (tag / #id / .class, no combinators).
class VIEWS_EXPORT CssParser {
 public:
  struct Rule {
    enum class SelectorKind {
      kTag,
      kId,
      kClass,
    };
    SelectorKind kind = SelectorKind::kTag;
    std::string name;
    std::vector<std::pair<std::string, std::string>> declarations;
  };

  bool Parse(std::string_view css, std::string* error = nullptr);
  void Clear();
  bool AppendRules(const CssParser& other);
  const std::vector<Rule>& rules() const { return rules_; }

  // Cascade: tag, then .class, then #id (document order inside each bucket).
  FlexStyle Resolve(std::string_view tag,
                    std::string_view id,
                    const std::vector<std::string>& classes) const;

  // Insert or replace declarations on an #id rule (editor-owned styles).
  void UpsertIdDeclarations(
      std::string_view id,
      const std::vector<std::pair<std::string, std::string>>& decls);

  std::string Serialize() const;

 private:
  std::vector<Rule> rules_;
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_STYLE_CSS_PARSER_H_
