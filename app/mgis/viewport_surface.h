// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_VIEWPORT_SURFACE_H_
#define APP_MGIS_VIEWPORT_SURFACE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/view.h"

namespace views {
class NativeViewHost;
}

namespace mgis {

// Center page that keeps one native child aligned with the view bounds.
class ViewportSurface : public views::View {
 public:
  ViewportSurface();
  virtual ~ViewportSurface();

  views::NativeViewHost* native_host() { return host_; }

  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void Layout() OVERRIDE;

 private:
  views::NativeViewHost* host_;

  DISALLOW_COPY_AND_ASSIGN(ViewportSurface);
};

}  // namespace mgis

#endif  // APP_MGIS_VIEWPORT_SURFACE_H_
