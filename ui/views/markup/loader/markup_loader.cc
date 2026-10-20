// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/loader/markup_loader.h"

#include <cctype>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/string16.h"
#include "base/synchronization/lock.h"
#include "base/utf_string_conversions.h"
#include "pugixml.hpp"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/font.h"
#include "ui/views/background.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/markup/factory/markup_combobox.h"
#include "ui/views/markup/factory/placeholder_view.h"
#include "ui/views/markup/layout/yoga_layout_manager.h"
#include "ui/views/view.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace views {

MarkupRoot::MarkupRoot() {}
MarkupRoot::MarkupRoot(MarkupRoot&&) noexcept = default;
MarkupRoot& MarkupRoot::operator=(MarkupRoot&&) noexcept = default;
MarkupRoot::~MarkupRoot() {}

bool MarkupRoot::ok() const {
  return root != NULL;
}

namespace {

std::string ToLower(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    out.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return out;
}

std::vector<std::string> SplitClasses(std::string_view raw) {
  std::vector<std::string> out;
  std::string cur;
  for (char c : raw) {
    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!cur.empty()) {
        out.push_back(cur);
        cur.clear();
      }
    } else {
      cur.push_back(c);
    }
  }
  if (!cur.empty())
    out.push_back(cur);
  return out;
}

bool IsFlexContainerTag(std::string_view tag) {
  const std::string t = ToLower(tag);
  return t == "flexrow" || t == "flexcolumn";
}

FlexStyle SugarDirection(std::string_view tag, FlexStyle style) {
  const std::string t = ToLower(tag);
  if (t == "flexrow") {
    if (!style.display.has_value())
      style.display = FlexStyle::Display::kFlex;
    if (!style.flex_direction.has_value())
      style.flex_direction = FlexStyle::FlexDirection::kRow;
  } else if (t == "flexcolumn") {
    if (!style.display.has_value())
      style.display = FlexStyle::Display::kFlex;
    if (!style.flex_direction.has_value())
      style.flex_direction = FlexStyle::FlexDirection::kColumn;
  }
  return style;
}

void ApplyPaintHints(View* view, const FlexStyle& style) {
  if (!view)
    return;
  if (style.background_color.has_value()) {
    view->set_background(
        Background::CreateSolidBackground(*style.background_color));
  }
  if (view->GetClassName() == Label::kViewClassName) {
    Label* label = static_cast<Label*>(view);
    if (style.color.has_value())
      label->SetEnabledColor(*style.color);
    if (style.font_size.has_value()) {
      const int px = static_cast<int>(*style.font_size);
      if (px > 0) {
        label->SetFont(gfx::Font(label->font().GetFontName(), px));
      }
    }
  }
}

MarkupAttrs AttrsFromNode(const pugi::xml_node& node) {
  MarkupAttrs attrs;
  for (pugi::xml_attribute a : node.attributes())
    attrs.values.insert(std::make_pair(std::string(a.name()),
                                        std::string(a.value())));
  return attrs;
}

bool IsSkippedChild(const pugi::xml_node& child) {
  if (child.type() != pugi::node_element)
    return true;
  const std::string name = ToLower(child.name());
  return name == "item" || name == "style";
}

void ApplyComboboxItems(View* view, const pugi::xml_node& node) {
  if (!view || view->GetClassName() != "views/MarkupCombobox")
    return;
  MarkupCombobox* combo = static_cast<MarkupCombobox*>(view);
  for (pugi::xml_node child : node.children()) {
    if (child.type() != pugi::node_element)
      continue;
    if (ToLower(child.name()) != "item")
      continue;
    const char* text = child.attribute("text").as_string("");
    combo->AddItem(UTF8ToUTF16(text));
  }
  if (combo->item_count() == 0)
    combo->AddItem(string16());
}

std::wstring Utf8ToWidePath(const std::string& u8) {
  if (u8.empty())
    return std::wstring();
  const int n = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, NULL, 0);
  if (n <= 0)
    return std::wstring();
  std::wstring w(static_cast<size_t>(n), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, &w[0], n);
  w.resize(static_cast<size_t>(n - 1));
  return w;
}

std::string WideToUtf8Path(const std::wstring& w) {
  if (w.empty())
    return std::string();
  const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL,
                                    NULL);
  if (n <= 0)
    return std::string();
  std::string u8(static_cast<size_t>(n), '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &u8[0], n, NULL, NULL);
  u8.resize(static_cast<size_t>(n - 1));
  return u8;
}

std::string ReadFile(const std::string& path) {
  const std::wstring wpath = Utf8ToWidePath(path);
  if (wpath.empty())
    return std::string();
  FILE* f = NULL;
  if (_wfopen_s(&f, wpath.c_str(), L"rb") != 0 || !f)
    return std::string();
  std::ostringstream ss;
  char buf[4096];
  while (const size_t n = fread(buf, 1, sizeof(buf), f))
    ss.write(buf, static_cast<std::streamsize>(n));
  fclose(f);
  return ss.str();
}

bool FileExists(const std::string& path) {
  const std::wstring wpath = Utf8ToWidePath(path);
  if (wpath.empty())
    return false;
  const DWORD attr = GetFileAttributesW(wpath.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::string ExeDir() {
  wchar_t buf[MAX_PATH] = {};
  const DWORD n = GetModuleFileNameW(NULL, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return std::string();
  std::wstring path(buf, n);
  const size_t slash = path.find_last_of(L"\\/");
  if (slash == std::wstring::npos)
    return std::string();
  return WideToUtf8Path(path.substr(0, slash));
}

std::string DirnameOf(const std::string& path) {
  const size_t slash = path.find_last_of("\\/");
  if (slash == std::string::npos)
    return std::string();
  return path.substr(0, slash);
}

std::string ResolveMarkupPathImpl(std::string_view path_or_name) {
  const std::string name(path_or_name);
  if (name.empty())
    return std::string();
  if (FileExists(name))
    return name;
  const std::string exe = ExeDir();
  const std::string candidates[] = {
      exe + "\\..\\..\\ui\\views\\markup\\testdata\\" + name,
      exe + "/../../ui/views/markup/testdata/" + name,
      exe + "\\" + name,
      exe + "/" + name,
      std::string("ui\\views\\markup\\testdata\\") + name,
      std::string("ui/views/markup/testdata/") + name,
  };
  for (const std::string& c : candidates) {
    if (FileExists(c))
      return c;
  }
  return std::string();
}

std::unique_ptr<View> BuildNode(const pugi::xml_node& node,
                                const CssParser& css,
                                const ControlFactory& factory,
                                NamedViewMap* ids,
                                std::string* error);

void AttachChildren(View* view,
                    YogaLayoutManager* yoga,
                    const std::vector<pugi::xml_node>& element_children,
                    const CssParser& css,
                    const ControlFactory& factory,
                    NamedViewMap* ids,
                    std::string* error) {
  for (size_t i = 0; i < element_children.size(); ++i) {
    const pugi::xml_node& child_xml = element_children[i];
    std::unique_ptr<View> child_view =
        BuildNode(child_xml, css, factory, ids, error);
    if (!child_view)
      return;
    MarkupAttrs child_attrs = AttrsFromNode(child_xml);
    FlexStyle child_style =
        css.Resolve(child_xml.name(), child_attrs.Get("id"),
                    SplitClasses(child_attrs.Get("class")));
    child_style = SugarDirection(child_xml.name(), child_style);
    View* raw = child_view.get();
    if (yoga)
      yoga->SetChildStyle(raw, child_style);
    view->AddChildView(child_view.release());
  }
}

std::unique_ptr<View> BuildNode(const pugi::xml_node& node,
                                const CssParser& css,
                                const ControlFactory& factory,
                                NamedViewMap* ids,
                                std::string* error) {
  const MarkupAttrs attrs = AttrsFromNode(node);
  std::unique_ptr<View> view = factory.Create(node.name(), attrs);
  if (!view) {
    const std::string caption =
        attrs.Get("id").empty() ? std::string(node.name()) : attrs.Get("id");
    view.reset(new PlaceholderView(caption));
  }

  ApplyComboboxItems(view.get(), node);

  const std::string id = attrs.Get("id");
  const std::vector<std::string> classes = SplitClasses(attrs.Get("class"));
  FlexStyle style = css.Resolve(node.name(), id, classes);
  style = SugarDirection(node.name(), style);
  ApplyPaintHints(view.get(), style);

  if (!id.empty())
    ids->Put(id, view.get());

  std::vector<pugi::xml_node> element_children;
  for (pugi::xml_node child : node.children()) {
    if (IsSkippedChild(child))
      continue;
    element_children.push_back(child);
  }

  const std::string tag = ToLower(node.name());
  if (tag == "scrollview") {
    ScrollView* scroll = static_cast<ScrollView*>(view.get());
    if (element_children.size() == 1) {
      std::unique_ptr<View> contents =
          BuildNode(element_children[0], css, factory, ids, error);
      if (!contents)
        return std::unique_ptr<View>();
      scroll->SetContents(contents.release());
    } else if (!element_children.empty()) {
      std::unique_ptr<View> column(new View());
      std::unique_ptr<YogaLayoutManager> yoga(new YogaLayoutManager());
      FlexStyle col;
      col.display = FlexStyle::Display::kFlex;
      col.flex_direction = FlexStyle::FlexDirection::kColumn;
      yoga->SetHostStyle(col);
      AttachChildren(column.get(), yoga.get(), element_children, css, factory,
                     ids, error);
      column->SetLayoutManager(yoga.release());
      scroll->SetContents(column.release());
    }
    return view;
  }

  if (!element_children.empty() || IsFlexContainerTag(node.name())) {
    std::unique_ptr<YogaLayoutManager> yoga(new YogaLayoutManager());
    yoga->SetHostStyle(style);
    AttachChildren(view.get(), yoga.get(), element_children, css, factory, ids,
                   error);
    view->SetLayoutManager(yoga.release());
  }

  (void)error;
  return view;
}

}  // namespace

std::string ResolveMarkupPath(std::string_view path_or_name) {
  return ResolveMarkupPathImpl(path_or_name);
}

bool BuildMarkupTree(const MarkupDocument& doc,
                     const MarkupOptions& options,
                     MarkupRoot* out) {
  if (!out)
    return false;
  out->root.reset();
  out->ids.Clear();
  out->error.clear();
  out->name = doc.Name();

  if (!doc.RawXml()) {
    out->error = "markup: null document";
    return false;
  }
  pugi::xml_node root_xml = doc.RawXml()->document_element();
  if (!root_xml) {
    out->error = "markup: no content root";
    return false;
  }

  const ControlFactory* factory = options.factory;
  if (!factory)
    factory = &ControlFactory::GetDefault();

  std::string err;
  out->root = BuildNode(root_xml, doc.Stylesheet(), *factory, &out->ids, &err);
  if (!out->root) {
    out->error = err.empty() ? "markup: build failed" : err;
    return false;
  }
  return true;
}

bool LoadMarkupBytes(std::string_view xml_utf8,
                     std::string_view base_dir,
                     const MarkupOptions& options,
                     MarkupRoot* out) {
  if (!out)
    return false;
  MarkupDocument doc;
  std::string err;
  if (!doc.LoadXml(xml_utf8, base_dir, &err)) {
    out->root.reset();
    out->error = err;
    return false;
  }
  return BuildMarkupTree(doc, options, out);
}

bool LoadMarkupInto(std::string_view path_or_name,
                    const MarkupOptions& options,
                    MarkupRoot* out) {
  if (!out)
    return false;
  *out = MarkupRoot();
  const std::string path = ResolveMarkupPathImpl(path_or_name);
  if (path.empty()) {
    out->error = std::string("markup: not found: ") + std::string(path_or_name);
    return false;
  }

  static base::Lock cache_lock;
  static std::unordered_map<std::string, std::string> xml_cache;
  std::string xml;
  {
    base::AutoLock guard(cache_lock);
    const auto it = xml_cache.find(path);
    if (it != xml_cache.end())
      xml = it->second;
  }
  if (xml.empty()) {
    xml = ReadFile(path);
    if (xml.empty()) {
      out->error = "markup: empty file: " + path;
      return false;
    }
    base::AutoLock guard(cache_lock);
    xml_cache.insert(std::make_pair(path, xml));
  }
  const std::string base = DirnameOf(path);
  if (!LoadMarkupBytes(xml, base, options, out)) {
    if (out->error.empty())
      out->error = "markup: load failed";
    return false;
  }
  return true;
}

MarkupRoot LoadMarkup(std::string_view path_or_name,
                      const MarkupOptions& options) {
  MarkupRoot out;
  LoadMarkupInto(path_or_name, options, &out);
  return out;
}

}  // namespace views
