// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_BOX_MAP_BOX_H
#define CONTENT_PUBLIC_MAP_BOX_MAP_BOX_H

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace content {
class MapBox : public base::RefCountedThreadSafe<MapBox>,
               public base::SupportsWeakPtr<MapBox> {
 public:
  struct Preferences {};

 public:
  // Property
  virtual void GetPreferences(MapBox::Preferences* preferences) = 0;
  virtual void GetVersion(std::string* version) = 0;

  // Create/Destroy
  virtual void Create(const MapBox::Preferences& preferences) = 0;
  virtual void Destroy() = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapBox>;
  virtual ~MapBox() {}
};
}  // namespace content
#endif  // CONTENT_PUBLIC_MAP_BOX_MAP_BOX_H
