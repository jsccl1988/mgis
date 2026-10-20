// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_VIEWPORT_PLACEHOLDER_H_
#define APP_MGIS_VIEWPORT_PLACEHOLDER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/string16.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/views/view.h"

namespace views {
class Label;
}

namespace mgis {

// Empty viewport page for the first chrome knife (no MapLibre/UE HWND yet).
class ViewportPlaceholder : public views::View {
 public:
  ViewportPlaceholder(const string16& caption, SkColor fill);
  virtual ~ViewportPlaceholder();

  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void Layout() OVERRIDE;

 private:
  string16 caption_;
  SkColor fill_;
  views::Label* label_;

  DISALLOW_COPY_AND_ASSIGN(ViewportPlaceholder);
};

}  // namespace mgis

#endif  // APP_MGIS_VIEWPORT_PLACEHOLDER_H_
