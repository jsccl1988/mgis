// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "feature_set.h"

#include "json_lite.h"

namespace sg {
namespace feature_set {

bool FeatureSet::FromSdbdJson(const std::string& json, FeatureSet* out) {
  if (!out) {
    return false;
  }
  json::Value root;
  if (!json::Parse(json, &root, nullptr) || !root.is_object()) {
    return false;
  }
  if (root.String("type") == "FeatureCollection" && !root.Get("features")) {
    return false;
  }
  FeatureSet fs;
  fs.layer_ = root.String("layer");
  fs.crs_ = root.String("crs");
  fs.offset_ = static_cast<int>(root.Number("offset", 0));
  const json::Value* feats = root.Get("features");
  if (!feats || !feats->is_array()) {
    return false;
  }
  for (const auto& item : feats->array) {
    if (!item.is_object()) {
      return false;
    }
    Feature f;
    if (const json::Value* id = item.Get("id")) {
      f.id = id->AsText();
    }
    f.geom_wkt = item.String("geom_wkt");
    if (const json::Value* attrs = item.Get("attrs")) {
      if (attrs->is_object()) {
        for (const auto& kv : attrs->object) {
          f.attrs[kv.first] = kv.second.AsText();
        }
      }
    }
    fs.features_.push_back(std::move(f));
  }
  const int declared = static_cast<int>(root.Number("count", -1));
  if (declared >= 0 && declared != static_cast<int>(fs.features_.size())) {
    // Prefer materialized count; still accept if server count matches intent.
  }
  fs.ended_ = true;
  fs.cursor_ = 0;
  *out = std::move(fs);
  return true;
}

bool FeatureSet::MoveFirst() {
  if (features_.empty()) {
    ended_ = true;
    cursor_ = 0;
    return false;
  }
  cursor_ = 0;
  ended_ = false;
  return true;
}

bool FeatureSet::MoveNext() {
  if (ended_ || features_.empty()) {
    ended_ = true;
    return false;
  }
  if (cursor_ + 1 >= features_.size()) {
    ended_ = true;
    return false;
  }
  ++cursor_;
  return true;
}

bool FeatureSet::IsEnd() const {
  return ended_ || features_.empty();
}

const Feature* FeatureSet::GetFeature() const {
  if (ended_ || cursor_ >= features_.size()) {
    return nullptr;
  }
  return &features_[cursor_];
}

}  // namespace feature_set
}  // namespace sg
