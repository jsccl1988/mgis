// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/feature_table.h"

#include <CesiumGltf/AccessorUtility.h>
#include <CesiumGltf/ExtensionModelExtStructuralMetadata.h>
#include <CesiumGltf/PropertyTableView.h>

#include <glm/vec3.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace cesium_flycube {
namespace {

std::int64_t ReadFeatureId(
    const CesiumGltf::FeatureIdAccessorType& accessor,
    std::int64_t index) {
  return std::visit(
      [&](const auto& view) -> std::int64_t {
        if (view.status() != CesiumGltf::AccessorViewStatus::Valid) {
          return -1;
        }
        CesiumGltf::FeatureIdFromAccessor visitor;
        visitor.index = index;
        return visitor(view);
      },
      accessor);
}

template <typename T>
bool CopyTypedColumn(CesiumGltf::PropertyTableView& view,
                     const std::string& property_id,
                     FeatureColumn* column) {
  auto values = view.getPropertyView<T>(property_id);
  if (values.status() != CesiumGltf::PropertyViewStatus::Valid) {
    return false;
  }
  column->property_id = property_id;
  column->values.assign(static_cast<size_t>(values.size()), std::string());
  for (std::int64_t i = 0; i < values.size(); ++i) {
    const auto value = values.get(i);
    if (value.has_value()) {
      if constexpr (std::is_same_v<T, std::string_view>) {
        column->values[static_cast<size_t>(i)] = std::string(*value);
      } else if constexpr (std::is_same_v<T, bool>) {
        column->values[static_cast<size_t>(i)] = *value ? "true" : "false";
      } else {
        column->values[static_cast<size_t>(i)] = std::to_string(*value);
      }
    }
  }
  return true;
}

void CopyColumns(const CesiumGltf::Model& model,
                 const CesiumGltf::PropertyTable& table,
                 FeatureBatch* batch) {
  CesiumGltf::PropertyTableView view(model, table);
  if (view.status() != CesiumGltf::PropertyTableViewStatus::Valid ||
      view.getClass() == nullptr) {
    return;
  }
  for (const auto& entry : view.getClass()->properties) {
    FeatureColumn column;
    if (CopyTypedColumn<std::string_view>(view, entry.first, &column) ||
        CopyTypedColumn<double>(view, entry.first, &column) ||
        CopyTypedColumn<float>(view, entry.first, &column) ||
        CopyTypedColumn<std::int64_t>(view, entry.first, &column) ||
        CopyTypedColumn<std::int32_t>(view, entry.first, &column) ||
        CopyTypedColumn<std::uint32_t>(view, entry.first, &column) ||
        CopyTypedColumn<bool>(view, entry.first, &column)) {
      batch->properties.push_back(std::move(column));
    }
  }
}

}  // namespace

FeatureBatch ExtractFeatureBatch(const CesiumGltf::Model& model,
                                 std::size_t vertex_count) {
  FeatureBatch batch;
  batch.feature_ids.assign(vertex_count, -1);
  std::size_t cursor = 0;
  for (const CesiumGltf::Mesh& mesh : model.meshes) {
    for (const CesiumGltf::MeshPrimitive& primitive : mesh.primitives) {
      const auto position = primitive.attributes.find("POSITION");
      if (position == primitive.attributes.end()) {
        continue;
      }
      CesiumGltf::AccessorView<glm::vec3> positions(model, position->second);
      if (positions.status() != CesiumGltf::AccessorViewStatus::Valid) {
        continue;
      }
      const CesiumGltf::FeatureIdAccessorType feature_ids =
          CesiumGltf::getFeatureIdAccessorView(model, primitive, 0);
      for (std::int64_t i = 0; i < positions.size(); ++i) {
        if (cursor >= batch.feature_ids.size()) {
          break;
        }
        batch.feature_ids[cursor] = ReadFeatureId(feature_ids, i);
        ++cursor;
      }
    }
  }

  const auto* metadata =
      model.getExtension<CesiumGltf::ExtensionModelExtStructuralMetadata>();
  if (metadata != nullptr) {
    for (const CesiumGltf::PropertyTable& table : metadata->propertyTables) {
      CopyColumns(model, table, &batch);
    }
  }
  return batch;
}

const std::string* PropertyForFeature(const std::vector<FeatureColumn>& columns,
                                      const std::string& property_id,
                                      std::int64_t feature_id) {
  if (feature_id < 0) {
    return nullptr;
  }
  for (const FeatureColumn& column : columns) {
    if (column.property_id != property_id) {
      continue;
    }
    const auto index = static_cast<size_t>(feature_id);
    if (index >= column.values.size() || column.values[index].empty()) {
      return nullptr;
    }
    return &column.values[index];
  }
  return nullptr;
}

}  // namespace cesium_flycube
