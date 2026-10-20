// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/layout/yoga_layout_manager.h"

#include <cmath>
#include <vector>

#include "base/basictypes.h"
#include "ui/views/view.h"
#include "yoga/Yoga.h"

namespace views {
namespace {

YGFlexDirection ToYgDirection(FlexStyle::FlexDirection d) {
  return d == FlexStyle::FlexDirection::kRow ? YGFlexDirectionRow
                                             : YGFlexDirectionColumn;
}

YGJustify ToYgJustify(FlexStyle::Justify j) {
  switch (j) {
    case FlexStyle::Justify::kFlexEnd:
      return YGJustifyFlexEnd;
    case FlexStyle::Justify::kCenter:
      return YGJustifyCenter;
    case FlexStyle::Justify::kSpaceBetween:
      return YGJustifySpaceBetween;
    case FlexStyle::Justify::kSpaceAround:
      return YGJustifySpaceAround;
    case FlexStyle::Justify::kSpaceEvenly:
      return YGJustifySpaceEvenly;
    case FlexStyle::Justify::kFlexStart:
    default:
      return YGJustifyFlexStart;
  }
}

YGAlign ToYgAlign(FlexStyle::Align a) {
  switch (a) {
    case FlexStyle::Align::kFlexStart:
      return YGAlignFlexStart;
    case FlexStyle::Align::kFlexEnd:
      return YGAlignFlexEnd;
    case FlexStyle::Align::kCenter:
      return YGAlignCenter;
    case FlexStyle::Align::kStretch:
      return YGAlignStretch;
    case FlexStyle::Align::kAuto:
    default:
      return YGAlignAuto;
  }
}

// Widget in this tree has no device_scale_factor. Keep markup lengths at 1.
float StyleScale(const View* view) {
  (void)view;
  return 1.f;
}

void ApplyFlexStyle(YGNodeRef node,
                    const FlexStyle& style,
                    bool is_host,
                    float scale) {
  if (style.display.has_value()) {
    YGNodeStyleSetDisplay(node, *style.display == FlexStyle::Display::kNone
                                    ? YGDisplayNone
                                    : YGDisplayFlex);
  }

  if (is_host) {
    if (style.flex_direction.has_value()) {
      YGNodeStyleSetFlexDirection(node,
                                  ToYgDirection(*style.flex_direction));
    }
    if (style.justify_content.has_value()) {
      YGNodeStyleSetJustifyContent(node, ToYgJustify(*style.justify_content));
    }
    if (style.align_items.has_value())
      YGNodeStyleSetAlignItems(node, ToYgAlign(*style.align_items));
    else
      YGNodeStyleSetAlignItems(node, YGAlignStretch);
    if (style.gap.has_value())
      YGNodeStyleSetGap(node, YGGutterAll, *style.gap * scale);
    const float pad = style.padding.value_or(0.f);
    YGNodeStyleSetPadding(node, YGEdgeLeft,
                          style.padding_left.value_or(pad) * scale);
    YGNodeStyleSetPadding(node, YGEdgeTop,
                          style.padding_top.value_or(pad) * scale);
    YGNodeStyleSetPadding(node, YGEdgeRight,
                          style.padding_right.value_or(pad) * scale);
    YGNodeStyleSetPadding(node, YGEdgeBottom,
                          style.padding_bottom.value_or(pad) * scale);
  }

  if (style.flex_grow.has_value())
    YGNodeStyleSetFlexGrow(node, *style.flex_grow);
  else if (style.flex.has_value())
    YGNodeStyleSetFlexGrow(node, *style.flex);
  if (style.flex_shrink.has_value())
    YGNodeStyleSetFlexShrink(node, *style.flex_shrink);
  if (style.flex.has_value())
    YGNodeStyleSetFlex(node, *style.flex);

  const float margin = style.margin.value_or(0.f);
  YGNodeStyleSetMargin(node, YGEdgeLeft,
                       style.margin_left.value_or(margin) * scale);
  YGNodeStyleSetMargin(node, YGEdgeTop,
                       style.margin_top.value_or(margin) * scale);
  YGNodeStyleSetMargin(node, YGEdgeRight,
                       style.margin_right.value_or(margin) * scale);
  YGNodeStyleSetMargin(node, YGEdgeBottom,
                       style.margin_bottom.value_or(margin) * scale);

  if (style.width.has_value())
    YGNodeStyleSetWidth(node, *style.width * scale);
  if (style.height.has_value())
    YGNodeStyleSetHeight(node, *style.height * scale);
  if (style.min_width.has_value())
    YGNodeStyleSetMinWidth(node, *style.min_width * scale);
  if (style.min_height.has_value())
    YGNodeStyleSetMinHeight(node, *style.min_height * scale);
  if (style.max_width.has_value())
    YGNodeStyleSetMaxWidth(node, *style.max_width * scale);
  if (style.max_height.has_value())
    YGNodeStyleSetMaxHeight(node, *style.max_height * scale);
}

YGSize MeasureView(YGNodeConstRef node,
                   float /*width*/,
                   YGMeasureMode /*widthMode*/,
                   float /*height*/,
                   YGMeasureMode /*heightMode*/) {
  View* view = static_cast<View*>(YGNodeGetContext(node));
  YGSize out;
  out.width = 0.f;
  out.height = 0.f;
  if (!view)
    return out;
  const gfx::Size pref = view->GetPreferredSize();
  out.width = static_cast<float>(pref.width());
  out.height = static_cast<float>(pref.height());
  return out;
}

int RoundPx(float v) {
  if (!std::isfinite(v))
    return 0;
  return static_cast<int>(std::lround(v));
}

struct YogaTree {
  YGNodeRef root;
  std::vector<YGNodeRef> children;

  YogaTree() : root(NULL) {}
  ~YogaTree() {
    if (root) {
      YGNodeFreeRecursive(root);
      root = NULL;
      children.clear();
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(YogaTree);
};

const FlexStyle* FindChildStyle(
    const std::unordered_map<const View*, FlexStyle>& map,
    const View* child) {
  const auto it = map.find(child);
  return it == map.end() ? NULL : &it->second;
}

bool HasGrow(const FlexStyle& style) {
  return (style.flex_grow.has_value() && *style.flex_grow > 0.f) ||
         (style.flex.has_value() && *style.flex > 0.f);
}

void BuildTree(View* host,
               const FlexStyle& host_style,
               const std::unordered_map<const View*, FlexStyle>& child_styles,
               YogaTree* tree,
               bool set_host_size,
               float scale) {
  tree->root = YGNodeNew();
  ApplyFlexStyle(tree->root, host_style, true, scale);
  if (set_host_size) {
    YGNodeStyleSetWidth(tree->root, static_cast<float>(host->width()));
    YGNodeStyleSetHeight(tree->root, static_cast<float>(host->height()));
  }

  for (int i = 0; i < host->child_count(); ++i) {
    View* child = host->child_at(i);
    if (!child->visible())
      continue;
    YGNodeRef yn = YGNodeNew();
    YGNodeSetContext(yn, child);
    const FlexStyle* cs = FindChildStyle(child_styles, child);
    FlexStyle style = cs ? *cs : FlexStyle();
    ApplyFlexStyle(yn, style, false, scale);
    const bool has_fixed_w = style.width.has_value();
    const bool has_fixed_h = style.height.has_value();
    if ((!has_fixed_w || !has_fixed_h) && !HasGrow(style))
      YGNodeSetMeasureFunc(yn, MeasureView);
    YGNodeInsertChild(tree->root, yn, YGNodeGetChildCount(tree->root));
    tree->children.push_back(yn);
  }
}

}  // namespace

YogaLayoutManager::YogaLayoutManager()
    : layout_host_(NULL),
      layout_w_(0),
      layout_h_(0),
      layout_valid_(false),
      pref_host_(NULL),
      pref_valid_(false),
      layouting_(false) {}

YogaLayoutManager::~YogaLayoutManager() {}

bool YogaLayoutManager::FlexLayoutEqual(const FlexStyle& a,
                                        const FlexStyle& b) const {
  return a.display == b.display && a.flex_direction == b.flex_direction &&
         a.justify_content == b.justify_content &&
         a.align_items == b.align_items && a.flex == b.flex &&
         a.flex_grow == b.flex_grow && a.flex_shrink == b.flex_shrink &&
         a.gap == b.gap && a.padding == b.padding &&
         a.padding_left == b.padding_left && a.padding_top == b.padding_top &&
         a.padding_right == b.padding_right &&
         a.padding_bottom == b.padding_bottom && a.margin == b.margin &&
         a.margin_left == b.margin_left && a.margin_top == b.margin_top &&
         a.margin_right == b.margin_right &&
         a.margin_bottom == b.margin_bottom && a.width == b.width &&
         a.height == b.height && a.min_width == b.min_width &&
         a.min_height == b.min_height && a.max_width == b.max_width &&
         a.max_height == b.max_height;
}

void YogaLayoutManager::InvalidateYogaCache() {
  layout_valid_ = false;
  pref_valid_ = false;
}

std::vector<YogaLayoutManager::ChildSnap>
YogaLayoutManager::CaptureChildSnaps(View* host) const {
  std::vector<ChildSnap> snaps;
  if (!host)
    return snaps;
  const int n = host->child_count();
  snaps.reserve(static_cast<size_t>(n));
  for (int i = 0; i < n; ++i) {
    View* child = host->child_at(i);
    ChildSnap snap;
    snap.child = child;
    if (!child) {
      snaps.push_back(snap);
      continue;
    }
    snap.visible = child->visible();
    if (snap.visible) {
      const FlexStyle* cs = FindChildStyle(child_styles_, child);
      FlexStyle style = cs ? *cs : FlexStyle();
      const bool has_fixed_w = style.width.has_value();
      const bool has_fixed_h = style.height.has_value();
      if ((!has_fixed_w || !has_fixed_h) && !HasGrow(style)) {
        const gfx::Size pref = child->GetPreferredSize();
        snap.pref_w = pref.width();
        snap.pref_h = pref.height();
      }
    }
    snaps.push_back(snap);
  }
  return snaps;
}

bool YogaLayoutManager::ChildSnapsEqual(
    const std::vector<ChildSnap>& a,
    const std::vector<ChildSnap>& b) const {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].child != b[i].child || a[i].visible != b[i].visible ||
        a[i].pref_w != b[i].pref_w || a[i].pref_h != b[i].pref_h) {
      return false;
    }
  }
  return true;
}

void YogaLayoutManager::SetHostStyle(const FlexStyle& style) {
  const bool layout_changed = !FlexLayoutEqual(host_style_, style);
  host_style_ = style;
  if (layout_changed)
    InvalidateYogaCache();
}

void YogaLayoutManager::SetChildStyle(const View* child,
                                      const FlexStyle& style) {
  if (!child)
    return;
  const auto it = child_styles_.find(child);
  if (it != child_styles_.end() && FlexLayoutEqual(it->second, style)) {
    it->second = style;
    return;
  }
  child_styles_[child] = style;
  InvalidateYogaCache();
}

void YogaLayoutManager::ClearChildStyle(const View* child) {
  if (child_styles_.erase(child) == 0)
    return;
  InvalidateYogaCache();
}

void YogaLayoutManager::Layout(View* host) {
  LayoutImpl(host);
}

gfx::Size YogaLayoutManager::GetPreferredSize(View* host) {
  return PreferredImpl(host);
}

void YogaLayoutManager::ViewRemoved(View* host, View* view) {
  (void)host;
  ClearChildStyle(view);
}

void YogaLayoutManager::LayoutImpl(View* host) {
  if (!host)
    return;
  // SetBoundsRect lays the child out immediately. Re-entering this manager
  // from that call must not rebuild the Yoga tree. Use an instance flag —
  // a process-wide static depth incorrectly interacts with nested hosts.
  if (layouting_)
    return;
  layouting_ = true;
  const float scale = StyleScale(host);
  std::vector<ChildSnap> snaps = CaptureChildSnaps(host);
  if (layout_valid_ && layout_host_ == host && layout_w_ == host->width() &&
      layout_h_ == host->height() && ChildSnapsEqual(layout_snaps_, snaps)) {
    layouting_ = false;
    return;
  }
  YogaTree tree;
  BuildTree(host, host_style_, child_styles_, &tree, true, scale);
  YGNodeCalculateLayout(tree.root,
                        static_cast<float>(host->width()),
                        static_cast<float>(host->height()),
                        YGDirectionLTR);
  for (YGNodeRef yn : tree.children) {
    View* child = static_cast<View*>(YGNodeGetContext(yn));
    if (!child)
      continue;
    // Child bounds are parent-local.
    child->SetBoundsRect(gfx::Rect(RoundPx(YGNodeLayoutGetLeft(yn)),
                                    RoundPx(YGNodeLayoutGetTop(yn)),
                                    RoundPx(YGNodeLayoutGetWidth(yn)),
                                    RoundPx(YGNodeLayoutGetHeight(yn))));
  }
  layout_host_ = host;
  layout_w_ = host->width();
  layout_h_ = host->height();
  layout_snaps_ = CaptureChildSnaps(host);
  layout_valid_ = true;
  layouting_ = false;
}

gfx::Size YogaLayoutManager::PreferredImpl(View* host) {
  if (!host)
    return gfx::Size();
  const float scale = StyleScale(host);
  std::vector<ChildSnap> snaps = CaptureChildSnaps(host);
  if (pref_valid_ && pref_host_ == host && ChildSnapsEqual(pref_snaps_, snaps))
    return pref_size_;
  YogaTree tree;
  BuildTree(host, host_style_, child_styles_, &tree, false, scale);
  YGNodeCalculateLayout(tree.root, YGUndefined, YGUndefined, YGDirectionLTR);
  pref_host_ = host;
  pref_snaps_ = CaptureChildSnaps(host);
  pref_size_ = gfx::Size(RoundPx(YGNodeLayoutGetWidth(tree.root)),
                         RoundPx(YGNodeLayoutGetHeight(tree.root)));
  pref_valid_ = true;
  return pref_size_;
}

}  // namespace views
