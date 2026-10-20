// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_FACTORY_PLACEHOLDER_VIEW_H_
#define UI_VIEWS_MARKUP_FACTORY_PLACEHOLDER_VIEW_H_

#include <string>

#include "base/basictypes.h"
#include "ui/views/view.h"
#include "ui/views/views_export.h"

namespace views {

// Dashed stand-in for unknown tags and editor slots. Preferred size is fixed
// so Yoga can measure the node without View::set_preferred_size.
class VIEWS_EXPORT PlaceholderView : public View {
 public:
  explicit PlaceholderView(std::string caption);
  virtual ~PlaceholderView();

  const std::string& caption() const { return caption_; }

  // View:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual std::string GetClassName() const OVERRIDE;

 protected:
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

 private:
  std::string caption_;

  DISALLOW_COPY_AND_ASSIGN(PlaceholderView);
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_FACTORY_PLACEHOLDER_VIEW_H_
