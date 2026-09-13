// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Display adapter only: FeatureSet WKT → OGR MEM. Not an sdbd driver.

#ifndef SDB_SDBD_OGR_MEM_H
#define SDB_SDBD_OGR_MEM_H

#include "core/sdb/sdbd_types.h"

#include <string>

class GDALDataset;
class OGRLayer;

namespace core {
namespace sdb {

class SdbdOgrMem {
 public:
  SdbdOgrMem();
  ~SdbdOgrMem();
  SdbdOgrMem(SdbdOgrMem&& other) noexcept;
  SdbdOgrMem& operator=(SdbdOgrMem&& other) noexcept;
  SdbdOgrMem(const SdbdOgrMem&) = delete;
  SdbdOgrMem& operator=(const SdbdOgrMem&) = delete;

  bool Load(const FeatureSet& set, const LayerInfo* schema, std::string* err);
  OGRLayer* layer() const;
  GDALDataset* dataset() const;
  bool empty() const { return layer_ == nullptr; }

 private:
  void Reset();

  GDALDataset* dataset_ = nullptr;
  OGRLayer* layer_ = nullptr;
};

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_OGR_MEM_H
