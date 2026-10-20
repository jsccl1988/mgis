// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/flycube_upload.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Instance/Instance.h"

#include <stdexcept>

namespace cesium_flycube {

FlyCubeUploadResult UploadToFlyCube(const std::vector<FlyCubeMesh>& meshes) {
  FlyCubeUploadResult result;
  const PackedMesh packed = PackMeshes(meshes);
  result.vertex_count = static_cast<std::uint32_t>(packed.xyz.size() / 3);
  result.index_count = static_cast<std::uint32_t>(packed.indices.size());
  if (result.vertex_count == 0 || result.index_count == 0) {
    result.message = "no mesh";
    return result;
  }

  try {
    std::shared_ptr<Instance> instance = CreateInstance(ApiType::kDX12);
    std::vector<std::shared_ptr<Adapter>> adapters =
        instance->EnumerateAdapters();
    if (adapters.empty()) {
      result.message = "no adapter";
      return result;
    }
    result.adapter_name = adapters.front()->GetName();
    std::shared_ptr<Device> device = adapters.front()->CreateDevice();

    const uint64_t vertex_bytes = packed.xyz.size() * sizeof(float);
    const uint64_t index_bytes = packed.indices.size() * sizeof(std::uint32_t);
    std::shared_ptr<Resource> vertices = device->CreateBuffer(
        MemoryType::kUpload, BufferDesc{vertex_bytes, BindFlag::kVertexBuffer});
    std::shared_ptr<Resource> index_buffer = device->CreateBuffer(
        MemoryType::kUpload, BufferDesc{index_bytes, BindFlag::kIndexBuffer});
    vertices->UpdateUploadBuffer(0, packed.xyz.data(), vertex_bytes);
    index_buffer->UpdateUploadBuffer(0, packed.indices.data(), index_bytes);
    result.ok = true;
    result.message = "uploaded";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

}  // namespace cesium_flycube
