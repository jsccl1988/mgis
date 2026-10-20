// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_LAYOUT_YOGA_LAYOUT_MANAGER_H_
#define UI_VIEWS_MARKUP_LAYOUT_YOGA_LAYOUT_MANAGER_H_

#include <unordered_map>
#include <vector>

#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "base/basictypes.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/markup/style/flex_style.h"
#include "ui/views/views_export.h"

namespace views {

// LayoutManager backed by Yoga. Positions direct children of the host.
// mview Widget has no device scale, so lengths stay at scale 1.
class VIEWS_EXPORT YogaLayoutManager : public LayoutManager {
 public:
  YogaLayoutManager();
  virtual ~YogaLayoutManager();

  void SetHostStyle(const FlexStyle& style);
  const FlexStyle& host_style() const { return host_style_; }

  void SetChildStyle(const View* child, const FlexStyle& style);
  void ClearChildStyle(const View* child);

  // LayoutManager:
  virtual void Layout(View* host) OVERRIDE;
  virtual gfx::Size GetPreferredSize(View* host) OVERRIDE;
  virtual void ViewRemoved(View* host, View* view) OVERRIDE;

 private:
  struct ChildSnap {
    const View* child;
    bool visible;
    int pref_w;
    int pref_h;
    ChildSnap() : child(NULL), visible(false), pref_w(0), pref_h(0) {}
  };

  std::vector<ChildSnap> CaptureChildSnaps(View* host) const;
  bool ChildSnapsEqual(const std::vector<ChildSnap>& a,
                       const std::vector<ChildSnap>& b) const;
  bool FlexLayoutEqual(const FlexStyle& a, const FlexStyle& b) const;
  void InvalidateYogaCache();
  void LayoutImpl(View* host);
  gfx::Size PreferredImpl(View* host);

  FlexStyle host_style_;
  std::unordered_map<const View*, FlexStyle> child_styles_;

  const View* layout_host_;
  int layout_w_;
  int layout_h_;
  std::vector<ChildSnap> layout_snaps_;
  bool layout_valid_;

  const View* pref_host_;
  std::vector<ChildSnap> pref_snaps_;
  gfx::Size pref_size_;
  bool pref_valid_;
  bool layouting_;

  DISALLOW_COPY_AND_ASSIGN(YogaLayoutManager);
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_LAYOUT_YOGA_LAYOUT_MANAGER_H_
