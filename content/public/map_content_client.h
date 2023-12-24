// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_CONTENT_CLIENT_H
#define CONTENT_PUBLIC_MAP_CONTENT_CLIENT_H

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "delegate/map_box_delegate.h"
#include "delegate/map_view_delegate.h"

namespace web_render {
class MapContentClient : public base::RefCountedThreadSafe<MapContentClient>,
                         public base::SupportsWeakPtr<MapContentClient> {
 public:
  virtual void QueryMapBoxDelegate(MapBoxDelegate** delegate) = 0;
  virtual void QueryMapViewDelegate(MapViewDelegate** delegate) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapContentClient>;
  virtual ~MapContentClient() {}
};
}  // namespace web_render
#endif  // CONTENT_PUBLIC_MAP_CONTENT_CLIENT_H
