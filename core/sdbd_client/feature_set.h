// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: matches target sg::feature_set::FeatureSet (04 §3.2).
// Display adapter may wrap as GeoJSON; this type remains FeatureSet.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace sg {
namespace feature_set {

struct Feature {
  std::string id;
  std::string geom_wkt;
  std::map<std::string, std::string> attrs;
};

class FeatureSet {
 public:
  static bool FromSdbdJson(const std::string& json, FeatureSet* out);

  bool MoveFirst();
  bool MoveNext();
  bool IsEnd() const;
  const Feature* GetFeature() const;

  const std::string& layer() const { return layer_; }
  const std::string& crs() const { return crs_; }
  int count() const { return static_cast<int>(features_.size()); }
  int offset() const { return offset_; }

 private:
  std::string layer_;
  std::string crs_;
  int offset_ = 0;
  std::vector<Feature> features_;
  std::size_t cursor_ = 0;
  bool ended_ = true;
};

}  // namespace feature_set
}  // namespace sg
