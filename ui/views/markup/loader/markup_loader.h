// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_LOADER_MARKUP_LOADER_H_
#define UI_VIEWS_MARKUP_LOADER_MARKUP_LOADER_H_

#include <memory>
#include <string>
#include <string_view>

#include "ui/views/markup/document/markup_document.h"
#include "ui/views/markup/document/named_view_map.h"
#include "ui/views/markup/factory/control_factory.h"
#include "ui/views/views_export.h"

namespace views {

class View;

struct VIEWS_EXPORT MarkupOptions {
  const ControlFactory* factory;
  MarkupOptions() : factory(NULL) {}
};

// Owns the loaded root View and the id map.
struct VIEWS_EXPORT MarkupRoot {
  MarkupRoot();
  MarkupRoot(MarkupRoot&&) noexcept;
  MarkupRoot& operator=(MarkupRoot&&) noexcept;
  ~MarkupRoot();
  MarkupRoot(const MarkupRoot&) = delete;
  MarkupRoot& operator=(const MarkupRoot&) = delete;

  std::unique_ptr<View> root;
  NamedViewMap ids;
  std::string name;
  std::string error;

  bool ok() const;
};

// Build a View tree from a MarkupDocument.
VIEWS_EXPORT bool BuildMarkupTree(const MarkupDocument& doc,
                                  const MarkupOptions& options,
                                  MarkupRoot* out);

// Parse XML bytes and build the tree.
VIEWS_EXPORT bool LoadMarkupBytes(std::string_view xml_utf8,
                                  std::string_view base_dir,
                                  const MarkupOptions& options,
                                  MarkupRoot* out);

// Load by path or basename (for example "preview_sample.ui.xml").
VIEWS_EXPORT bool LoadMarkupInto(std::string_view path_or_name,
                                 const MarkupOptions& options,
                                 MarkupRoot* out);

VIEWS_EXPORT MarkupRoot LoadMarkup(std::string_view path_or_name,
                                   const MarkupOptions& options = MarkupOptions());

// Resolve a markup path or basename to an existing file path (UTF-8).
VIEWS_EXPORT std::string ResolveMarkupPath(std::string_view path_or_name);

}  // namespace views

#endif  // UI_VIEWS_MARKUP_LOADER_MARKUP_LOADER_H_
