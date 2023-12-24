// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_DELEGATE_MAP_VIEW_DELEGATE_H
#define CONTENT_PUBLIC_DELEGATE_MAP_VIEW_DELEGATE_H

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace content {
class MapViewDelegate : public base::RefCountedThreadSafe<MapViewDelegate>,
                        public base::SupportsWeakPtr<MapViewDelegate> {
 public:
  virtual void OnStatusChange(const std::string &status) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapViewDelegate>;
  virtual ~MapViewDelegate() {}
};
}  // namespace content
#endif  // CONTENT_PUBLIC_DELEGATE_MAP_VIEW_DELEGATE_H
