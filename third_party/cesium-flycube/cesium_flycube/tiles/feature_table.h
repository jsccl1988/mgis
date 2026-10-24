// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include <CesiumGltf/Model.h>

#include <cstdint>
#include <string>
#include <vector>

namespace cesium_flycube {

// One EXT_structural_metadata column, indexed by feature id.
struct FeatureColumn {
  std::string property_id;
  std::vector<std::string> values;
};

struct FeatureBatch {
  std::vector<std::int64_t> feature_ids;
  std::vector<FeatureColumn> properties;
};

// Reads _FEATURE_ID_0 and property-table columns (string, scalar, bool).
FeatureBatch ExtractFeatureBatch(const CesiumGltf::Model& model,
                                 std::size_t vertex_count);

const std::string* PropertyForFeature(const std::vector<FeatureColumn>& columns,
                                      const std::string& property_id,
                                      std::int64_t feature_id);

}  // namespace cesium_flycube
