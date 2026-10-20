// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/tile_mesh.h"

#include <CesiumGltf/AccessorView.h>

#include <glm/vec4.hpp>

namespace cesium_flycube {
namespace {

glm::dvec3 TransformPoint(const glm::dmat4& transform, const glm::vec3& point) {
  const glm::dvec4 transformed =
      transform * glm::dvec4(point.x, point.y, point.z, 1.0);
  return glm::dvec3(transformed);
}

template <typename Index>
void AppendIndices(const CesiumGltf::Model& model,
                   int accessor_index,
                   std::uint32_t vertex_base,
                   std::vector<std::uint32_t>* indices) {
  CesiumGltf::AccessorView<Index> view(model, accessor_index);
  if (view.status() != CesiumGltf::AccessorViewStatus::Valid) {
    return;
  }
  indices->reserve(indices->size() + static_cast<size_t>(view.size()));
  for (int64_t i = 0; i < view.size(); ++i) {
    indices->push_back(vertex_base + static_cast<std::uint32_t>(view[i]));
  }
}

}  // namespace

TileMesh ExtractTileMesh(const CesiumGltf::Model& model,
                         const glm::dmat4& tile_to_ecef) {
  TileMesh mesh;
  for (const CesiumGltf::Mesh& gltf_mesh : model.meshes) {
    for (const CesiumGltf::MeshPrimitive& primitive : gltf_mesh.primitives) {
      const auto position = primitive.attributes.find("POSITION");
      if (position == primitive.attributes.end()) {
        continue;
      }
      CesiumGltf::AccessorView<glm::vec3> positions(model, position->second);
      if (positions.status() != CesiumGltf::AccessorViewStatus::Valid ||
          positions.size() <= 0) {
        continue;
      }

      const auto vertex_base =
          static_cast<std::uint32_t>(mesh.ecef_positions.size());
      mesh.ecef_positions.reserve(mesh.ecef_positions.size() +
                                  static_cast<size_t>(positions.size()));
      for (int64_t i = 0; i < positions.size(); ++i) {
        mesh.ecef_positions.push_back(
            TransformPoint(tile_to_ecef, positions[i]));
      }

      const size_t index_before = mesh.indices.size();
      if (primitive.indices >= 0) {
        AppendIndices<std::uint16_t>(
            model, primitive.indices, vertex_base, &mesh.indices);
        if (mesh.indices.size() == index_before) {
          AppendIndices<std::uint32_t>(
              model, primitive.indices, vertex_base, &mesh.indices);
        }
      }
      if (mesh.indices.size() == index_before) {
        for (int64_t i = 0; i < positions.size(); ++i) {
          mesh.indices.push_back(vertex_base + static_cast<std::uint32_t>(i));
        }
      }
    }
  }
  return mesh;
}

}  // namespace cesium_flycube
