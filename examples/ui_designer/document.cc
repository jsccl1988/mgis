// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "examples/ui_designer/document.h"

#include <string>

#include "examples/ui_designer/io.h"

namespace ui_designer {
namespace {

bool IsStyleTag(const pugi::xml_node& node) {
  return ToLowerCopy(node.name()) == "style";
}

bool TagShowsText(const std::string& tag) {
  const std::string lower = ToLowerCopy(tag);
  return lower == "label" || lower == "labelbutton" || lower == "textfield" ||
         lower == "checkbox" || lower == "radiobutton" ||
         lower == "placeholder";
}

bool IdTaken(const pugi::xml_node& node, const std::string& id) {
  if (!node)
    return false;
  if (AttributeOf(node, "id") == id)
    return true;
  for (pugi::xml_node child = node.first_child(); child;
       child = child.next_sibling()) {
    if (child.type() == pugi::node_element && IdTaken(child, id))
      return true;
  }
  return false;
}

std::string MakeId(const pugi::xml_document& doc, const std::string& tag) {
  const std::string prefix = ToLowerCopy(tag);
  const pugi::xml_node root = doc.document_element();
  for (int n = 1; n < 10000; ++n) {
    const std::string id = prefix + "_" + std::to_string(n);
    if (!IdTaken(root, id))
      return id;
  }
  return prefix + "_x";
}

void CollectFrom(const pugi::xml_node& node,
                 int depth,
                 std::vector<TreeEntry>* out) {
  if (!node || node.type() != pugi::node_element)
    return;
  TreeEntry entry;
  entry.node = node;
  entry.depth = depth;
  entry.label = std::string(static_cast<size_t>(depth) * 2, ' ');
  entry.label += node.name();
  const std::string id = AttributeOf(node, "id");
  if (!id.empty()) {
    entry.label += " #";
    entry.label += id;
  }
  out->push_back(entry);
  for (pugi::xml_node child = node.first_child(); child;
       child = child.next_sibling()) {
    if (child.type() == pugi::node_element)
      CollectFrom(child, depth + 1, out);
  }
}

}  // namespace

void CollectElements(const pugi::xml_document* doc,
                     std::vector<TreeEntry>* out) {
  if (!doc || !out)
    return;
  out->clear();
  CollectFrom(doc->document_element(), 0, out);
}

std::string AttributeOf(const pugi::xml_node& node, const char* name) {
  if (!node || !name)
    return std::string();
  return node.attribute(name).as_string("");
}

void SetAttribute(pugi::xml_node node,
                  const char* name,
                  const std::string& value) {
  if (!node || !name)
    return;
  pugi::xml_attribute attr = node.attribute(name);
  if (value.empty()) {
    if (attr)
      node.remove_attribute(attr);
    return;
  }
  if (!attr)
    attr = node.append_attribute(name);
  attr.set_value(value.c_str());
}

pugi::xml_node InsertElement(pugi::xml_document* doc,
                             pugi::xml_node selected,
                             const std::string& tag) {
  if (!doc || tag.empty())
    return pugi::xml_node();
  pugi::xml_node parent = selected;
  if (!parent)
    parent = doc->document_element();
  if (parent && IsStyleTag(parent))
    parent = parent.parent();
  if (!parent)
    return pugi::xml_node();

  pugi::xml_node child = parent.append_child(tag.c_str());
  const std::string id = MakeId(*doc, tag);
  SetAttribute(child, "id", id);
  if (TagShowsText(tag))
    SetAttribute(child, "text", tag);
  if (ToLowerCopy(tag) == "radiobutton")
    SetAttribute(child, "group", "1");
  if (ToLowerCopy(tag) == "combobox") {
    pugi::xml_node item = child.append_child("item");
    SetAttribute(item, "text", "Item");
  }
  return child;
}

std::string StylesheetPath(views::MarkupDocument* doc,
                           const std::string& xml_path) {
  if (!doc || !doc->RawXml() || xml_path.empty())
    return std::string();
  pugi::xml_node root = doc->RawXml()->document_element();
  for (pugi::xml_node child = root.first_child(); child;
       child = child.next_sibling()) {
    if (!IsStyleTag(child))
      continue;
    const char* src = child.attribute("src").as_string(NULL);
    if (!src || src[0] == '\0')
      continue;
    const std::string base = DirnameOf(xml_path);
    if (base.empty())
      return src;
    return base + "\\" + src;
  }

  std::string css_path = xml_path;
  const std::string xml_ext = ".xml";
  if (css_path.size() >= xml_ext.size() &&
      ToLowerCopy(css_path.substr(css_path.size() - xml_ext.size())) ==
          xml_ext) {
    css_path.replace(css_path.size() - xml_ext.size(), xml_ext.size(), ".css");
  } else {
    css_path += ".ui.css";
  }

  pugi::xml_node style;
  for (pugi::xml_node child = root.first_child(); child;
       child = child.next_sibling()) {
    if (IsStyleTag(child)) {
      style = child;
      break;
    }
  }
  if (!style)
    style = root.prepend_child("Style");
  SetAttribute(style, "src", BasenameOf(css_path));
  return css_path;
}

}  // namespace ui_designer
