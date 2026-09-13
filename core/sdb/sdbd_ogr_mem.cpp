// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "core/sdb/sdbd_ogr_mem.h"
#include "core/sdb/ogr_bind_traits.h"

#include <ogrsf_frmts.h>

#include <utility>
#include <vector>

namespace core {
namespace sdb {
namespace {

OGRwkbGeometryType geom_type_of(const LayerInfo* schema, const FeatureSet& set) {
  std::string type;
  if (schema && !schema->geom_type.empty()) {
    type = schema->geom_type;
  }
  const OGRwkbGeometryType mapped = ogr_bind_traits::geom_type_of(type);
  if (mapped != wkbUnknown) {
    return mapped;
  }
  if (!set.features.empty() &&
      set.features.front().geom_wkt.rfind("POLYGON", 0) == 0) {
    return wkbPolygon;
  }
  return wkbUnknown;
}

}  // namespace

SdbdOgrMem::SdbdOgrMem() = default;

SdbdOgrMem::~SdbdOgrMem() { Reset(); }

SdbdOgrMem::SdbdOgrMem(SdbdOgrMem&& other) noexcept
    : dataset_(other.dataset_), layer_(other.layer_) {
  other.dataset_ = nullptr;
  other.layer_ = nullptr;
}

SdbdOgrMem& SdbdOgrMem::operator=(SdbdOgrMem&& other) noexcept {
  if (this != &other) {
    Reset();
    dataset_ = other.dataset_;
    layer_ = other.layer_;
    other.dataset_ = nullptr;
    other.layer_ = nullptr;
  }
  return *this;
}

OGRLayer* SdbdOgrMem::layer() const { return layer_; }

GDALDataset* SdbdOgrMem::dataset() const { return dataset_; }

void SdbdOgrMem::Reset() {
  layer_ = nullptr;
  if (dataset_) {
    GDALClose(dataset_);
    dataset_ = nullptr;
  }
}

bool SdbdOgrMem::Load(const FeatureSet& set, const LayerInfo* schema,
                      std::string* err) {
  Reset();
  const std::string layer_name =
      set.layer.empty() ? std::string(kDemoLayer) : set.layer;
  dataset_ = create_memory_dataset(layer_name);
  if (!dataset_) {
    if (err) {
      *err = "failed to create Memory dataset";
    }
    return false;
  }

  layer_ = dataset_->CreateLayer(layer_name.c_str(), nullptr,
                                 geom_type_of(schema, set), nullptr);
  if (!layer_) {
    if (err) {
      *err = "failed to create Memory layer";
    }
    Reset();
    return false;
  }

  OGRFieldDefn id_field("id", OFTString);
  id_field.SetWidth(32);
  layer_->CreateField(&id_field);
  if (schema) {
    for (const auto& field : schema->fields) {
      if (field.name == "id") {
        continue;
      }
      OGRFieldDefn defn(field.name.c_str(),
                        ogr_bind_traits::field_type_of(field.type));
      defn.SetWidth(64);
      layer_->CreateField(&defn);
    }
  } else if (!set.features.empty()) {
    for (const auto& kv : set.features.front().attrs) {
      OGRFieldDefn defn(kv.first.c_str(), OFTString);
      defn.SetWidth(64);
      layer_->CreateField(&defn);
    }
  }

  for (const auto& feature : set.features) {
    if (feature.geom_wkt.empty()) {
      continue;
    }
    std::vector<char> wkt(feature.geom_wkt.begin(), feature.geom_wkt.end());
    wkt.insert(wkt.end(), 8, '\0');
    char* wkt_ptr = wkt.data();
    OGRGeometry* raw = nullptr;
    if (OGRGeometryFactory::createFromWkt(&wkt_ptr, nullptr, &raw) !=
        OGRERR_NONE) {
      if (raw) {
        OGRGeometryFactory::destroyGeometry(raw);
      }
      if (err) {
        *err = "bad_request";
      }
      Reset();
      return false;
    }
    OGRFeature* row = OGRFeature::CreateFeature(layer_->GetLayerDefn());
    row->SetGeometryDirectly(raw);
    row->SetField("id", feature.id.c_str());
    for (const auto& kv : feature.attrs) {
      row->SetField(kv.first.c_str(), kv.second.c_str());
    }
    if (layer_->CreateFeature(row) != OGRERR_NONE) {
      OGRFeature::DestroyFeature(row);
      if (err) {
        *err = "failed to add feature";
      }
      Reset();
      return false;
    }
    OGRFeature::DestroyFeature(row);
  }
  return true;
}

}  // namespace sdb
}  // namespace core
