// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_FRAME_MAP_VIEW_H
#define CONTENT_PUBLIC_FRAME_MAP_VIEW_H

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/map_types.h"

namespace content {
class MapView : public base::RefCountedThreadSafe<MapView>,
                public base::SupportsWeakPtr<MapView> {
 public:
  struct CreateParams {
    CreateParams() {}
    CreateParams(const CreateParams& other) { this->operator=(other); }

    CreateParams& operator=(const CreateParams& other) {
      if (this == &other) return *this;

      return *this;
    }
  };

  struct Preferences {
    Preferences() {}
    Preferences(const Preferences& other) { this->operator=(other); }

    Preferences& operator=(const Preferences& other) {
      if (this == &other) return *this;
      return *this;
    }
  };

 public:
  // Property
  virtual void GetCreateParams(MapView::CreateParams* params) = 0;
  virtual void GetPreferences(MapView::Preferences* preferences) = 0;
  virtual void GetViewID(uint32_t* id) = 0;

  // Create/Destroy
  virtual void Create(const MapView::CreateParams& params,
                      const MapView::Preferences& preferences) = 0;
  virtual void Destroy() = 0;

  // View Control
  virtual void Resize(const Rect& rect) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapView>;
  virtual ~MapView() {}
};
}  // namespace content
#endif  // CONTENT_PUBLIC_FRAME_MAP_VIEW_H
