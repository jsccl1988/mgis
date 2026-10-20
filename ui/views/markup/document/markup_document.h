// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_DOCUMENT_MARKUP_DOCUMENT_H_
#define UI_VIEWS_MARKUP_DOCUMENT_MARKUP_DOCUMENT_H_

#include <memory>
#include <string>
#include <string_view>

#include "ui/views/markup/style/css_parser.h"
#include "ui/views/views_export.h"

namespace pugi {
class xml_document;
}  // namespace pugi

namespace views {

// Parsed .ui.xml plus linked or inline CSS (Flex subset).
class VIEWS_EXPORT MarkupDocument {
 public:
  MarkupDocument();
  ~MarkupDocument();

  MarkupDocument(const MarkupDocument&) = delete;
  MarkupDocument& operator=(const MarkupDocument&) = delete;

  bool LoadXml(std::string_view xml_utf8,
               std::string_view base_dir = {},
               std::string* error = nullptr);
  bool LoadCss(std::string_view css_utf8, std::string* error = nullptr);

  const CssParser& Stylesheet() const { return css_; }
  CssParser& Stylesheet() { return css_; }
  std::string Name() const;

  // Mutable XML for UiDesigner edits.
  pugi::xml_document* RawXml() { return doc_.get(); }
  const pugi::xml_document* RawXml() const { return doc_.get(); }

  std::string SerializeXml() const;
  std::string SerializeCss() const;
  bool SaveFiles(const std::string& xml_path,
                 const std::string& css_path) const;

 private:
  bool LoadStyleLinks(std::string_view base_dir, std::string* error);

  std::unique_ptr<pugi::xml_document> doc_;
  CssParser css_;
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_DOCUMENT_MARKUP_DOCUMENT_H_
