// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef EXAMPLES_UI_DESIGNER_DOCUMENT_H_
#define EXAMPLES_UI_DESIGNER_DOCUMENT_H_

#include <string>
#include <vector>

#include "pugixml.hpp"
#include "ui/views/markup/document/markup_document.h"

namespace ui_designer {

struct TreeEntry {
  pugi::xml_node node;
  int depth;
  std::string label;
};

void CollectElements(const pugi::xml_document* doc, std::vector<TreeEntry>* out);

std::string AttributeOf(const pugi::xml_node& node, const char* name);
void SetAttribute(pugi::xml_node node, const char* name, const std::string& value);

// Append |tag| under |selected|, or under the document element when
// |selected| is empty. Returns the new node.
pugi::xml_node InsertElement(pugi::xml_document* doc,
                             pugi::xml_node selected,
                             const std::string& tag);

// Linked Style src, or a sibling .ui.css path recorded on the document.
std::string StylesheetPath(views::MarkupDocument* doc,
                           const std::string& xml_path);

}  // namespace ui_designer

#endif  // EXAMPLES_UI_DESIGNER_DOCUMENT_H_
