// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_DELEGATE_MAP_BOX_DELEGATE_H
#define CONTENT_PUBLIC_DELEGATE_MAP_BOX_DELEGATE_H

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace content {
class MapBoxDelegate : public base::RefCountedThreadSafe<MapBoxDelegate>,
                         public base::SupportsWeakPtr<MapBoxDelegate> {
 public:
  virtual void PreMainMessageLoopRun() = 0;
  virtual void OnVersionChange(const std::string& version) = 0;

 protected:
  friend class base::RefCountedThreadSafe<MapBoxDelegate>;
  virtual ~MapBoxDelegate() {}
};
}  // namespace content
#endif  // CONTENT_PUBLIC_DELEGATE_MAP_BOX_DELEGATE_H
