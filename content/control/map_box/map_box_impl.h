// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_MAP_BOX_MAP_BOX_IMPL_H
#define CONTENT_CONTROL_MAP_BOX_MAP_BOX_IMPL_H

#include <string>

#include "content/public/map_box/map_view.h"

namespace content {
class CONTENT_EXPORT MapBoxImpl : public MapBox {
 public:
  MapBoxImpl() {}
  virtual ~MapBoxImpl() {}

 public:
  // Property
  virtual void GetPreferences(MapBox::Preferences* preferences) {}
  virtual void GetVersion(std::string* version) {}

  // Create/Destroy
  virtual void Create(const MapBox::Preferences& preferences) {}
  virtual void Destroy() {}
};
}  // namespace content
#endif  // CONTENT_CONTROL_MAP_BOX_MAP_BOX_IMPL_H
