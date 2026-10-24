// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/tile_mesh.h"

#include <CesiumGltf/AccessorView.h>
#include <CesiumRasterOverlays/RasterOverlayUtilities.h>

#include <glm/mat3x3.hpp>
#include <glm/vec4.hpp>

#include <string>

namespace cesium_flycube {
namespace {

glm::dvec3 TransformPoint(const glm::dmat4& transform, const glm::vec3& point) {
  const glm::dvec4 transformed =
      transform * glm::dvec4(point.x, point.y, point.z, 1.0);
  return glm::dvec3(transformed);
}

glm::vec3 TransformNormal(const glm::dmat3& normal_matrix,
                          const glm::vec3& normal) {
  const glm::dvec3 transformed =
      normal_matrix * glm::dvec3(normal.x, normal.y, normal.z);
  const double length = glm::length(transformed);
  if (length < 1.0e-12) {
    return glm::vec3(0.0f, 0.0f, 1.0f);
  }
  const glm::dvec3 unit = transformed / length;
  return glm::vec3(static_cast<float>(unit.x), static_cast<float>(unit.y),
                   static_cast<float>(unit.z));
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

int FindUvAccessor(const CesiumGltf::MeshPrimitive& primitive) {
  const std::string overlay0 =
      std::string(CesiumRasterOverlays::RasterOverlayUtilities::
                      DEFAULT_TEXTURE_COORDINATE_BASE_NAME) +
      "0";
  const auto overlay = primitive.attributes.find(overlay0);
  if (overlay != primitive.attributes.end()) {
    return overlay->second;
  }
  const auto texcoord = primitive.attributes.find("TEXCOORD_0");
  if (texcoord != primitive.attributes.end()) {
    return texcoord->second;
  }
  return -1;
}

}  // namespace

TileMesh ExtractTileMesh(const CesiumGltf::Model& model,
                         const glm::dmat4& tile_to_ecef) {
  TileMesh mesh;
  const glm::dmat3 normal_matrix = glm::dmat3(tile_to_ecef);
  bool any_uv = false;
  bool any_normal = false;

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

      const int uv_accessor = FindUvAccessor(primitive);
      bool have_uv = false;
      if (uv_accessor >= 0) {
        CesiumGltf::AccessorView<glm::vec2> uvs(model, uv_accessor);
        have_uv = uvs.status() == CesiumGltf::AccessorViewStatus::Valid &&
                  uvs.size() == positions.size();
        if (have_uv) {
          any_uv = true;
          mesh.texcoords.reserve(mesh.texcoords.size() +
                                 static_cast<size_t>(uvs.size()));
          for (int64_t i = 0; i < uvs.size(); ++i) {
            mesh.texcoords.push_back(uvs[i]);
          }
        }
      }
      if (!have_uv) {
        mesh.texcoords.resize(mesh.ecef_positions.size(),
                              glm::vec2(0.5f, 0.5f));
      }

      const auto normal = primitive.attributes.find("NORMAL");
      bool have_normal = false;
      if (normal != primitive.attributes.end()) {
        CesiumGltf::AccessorView<glm::vec3> normals(model, normal->second);
        have_normal =
            normals.status() == CesiumGltf::AccessorViewStatus::Valid &&
            normals.size() == positions.size();
        if (have_normal) {
          any_normal = true;
          if (mesh.normals.size() < static_cast<size_t>(vertex_base)) {
            mesh.normals.resize(vertex_base, glm::vec3(0.0f, 0.0f, 1.0f));
          }
          mesh.normals.reserve(mesh.normals.size() +
                               static_cast<size_t>(normals.size()));
          for (int64_t i = 0; i < normals.size(); ++i) {
            mesh.normals.push_back(TransformNormal(normal_matrix, normals[i]));
          }
        }
      }
      if (!have_normal && any_normal) {
        mesh.normals.resize(mesh.ecef_positions.size(),
                            glm::vec3(0.0f, 0.0f, 1.0f));
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

  if (!any_uv) {
    mesh.texcoords.clear();
  }
  if (!any_normal) {
    mesh.normals.clear();
  }
  mesh.features = ExtractFeatureBatch(model, mesh.ecef_positions.size());
  return mesh;
}

std::int64_t FeatureIdAt(const TileMesh& mesh, std::size_t index) {
  if (index >= mesh.features.feature_ids.size()) {
    return -1;
  }
  return mesh.features.feature_ids[index];
}

}  // namespace cesium_flycube
