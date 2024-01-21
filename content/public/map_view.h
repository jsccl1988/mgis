// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_VIEW_H
#define CONTENT_PUBLIC_MAP_VIEW_H

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/map_types.h"

namespace content {
class MapView : public base::RefCountedThreadSafe<MapView>,
                public base::SupportsWeakPtr<MapView> {
 public:
  struct CreateParams {
    HWND parent_hwnd{nullptr};
  };

  struct Preferences {};

 public:
  // Create/Destroy
  virtual void Create(const MapView::CreateParams& params,
                      const MapView::Preferences& preferences) = 0;
  virtual void Destroy() = 0;

  // Property
  virtual void GetCreateParams(MapView::CreateParams* params) = 0;
  virtual void GetPreferences(MapView::Preferences* preferences) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapView>;
  virtual ~MapView() {}
};
}  // namespace content
#endif  // CONTENT_PUBLIC_MAP_VIEW_H
