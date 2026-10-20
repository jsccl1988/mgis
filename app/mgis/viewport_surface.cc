// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/viewport_surface.h"

#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/native/native_view_host.h"

namespace mgis {

ViewportSurface::ViewportSurface() : host_(NULL) {
  host_ = new views::NativeViewHost();
  AddChildView(host_);
}

ViewportSurface::~ViewportSurface() {}

void ViewportSurface::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SkColorSetRGB(0xe4, 0xea, 0xf0));
  views::View::OnPaint(canvas);
}

void ViewportSurface::Layout() {
  if (host_)
    host_->SetBounds(0, 0, width(), height());
}

}  // namespace mgis
