// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/scene_present.h"

#include "cesium_flycube/view/camera_flight.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Instance/Instance.h"

#include <glm/vec4.hpp>

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <vector>

namespace cesium_flycube {
namespace {

constexpr int kWidth = 960;
constexpr int kHeight = 540;
constexpr int kFrames = 30;

struct Vec3 {
  float x;
  float y;
  float z;
};

struct NdcVertex {
  float x;
  float y;
  float depth;
  bool visible;
  float r;
  float g;
  float b;
};

LRESULT CALLBACK PresentWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_CLOSE) {
    DestroyWindow(hwnd);
    return 0;
  }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}

Vec3 Sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

float Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 Cross(Vec3 a, Vec3 b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 Normalize(Vec3 v) {
  const float length = std::sqrt(Dot(v, v));
  if (length < 1e-8f) {
    return {0, 0, 1};
  }
  return {v.x / length, v.y / length, v.z / length};
}

struct Viewport {
  int x = 0;
  int y = 0;
  int width = kWidth;
  int height = kHeight;
};

NdcVertex Project(const PresentCamera& camera, Vec3 point, int view_width, int view_height) {
  const Vec3 eye{camera.eye[0], camera.eye[1], camera.eye[2]};
  const Vec3 target{camera.target[0], camera.target[1], camera.target[2]};
  const Vec3 up{camera.up[0], camera.up[1], camera.up[2]};
  const Vec3 forward = Normalize(Sub(target, eye));
  const Vec3 side = Normalize(Cross(forward, up));
  const Vec3 camera_up = Cross(side, forward);
  const Vec3 offset = Sub(point, eye);
  const float view_x = Dot(side, offset);
  const float view_y = Dot(camera_up, offset);
  const float view_z = Dot(forward, offset);
  if (view_z <= 1.0f) {
    return {0, 0, 1.0e30f, false, 0, 0, 0};
  }
  const float fy = 1.0f / std::tan(camera.fov_y_radians * 0.5f);
  const float fx = fy / (static_cast<float>(view_width) / static_cast<float>(view_height));
  return {(fx * view_x) / view_z, (fy * view_y) / view_z, view_z, true, 1, 1, 1};
}

float ViewportNdcX(float ndc_x, const Viewport& viewport) {
  const float pixel = (ndc_x * 0.5f + 0.5f) * static_cast<float>(viewport.width) +
                      static_cast<float>(viewport.x);
  return (pixel / static_cast<float>(kWidth)) * 2.0f - 1.0f;
}

float ViewportNdcY(float ndc_y, const Viewport& viewport) {
  const float pixel = (1.0f - (ndc_y * 0.5f + 0.5f)) * static_cast<float>(viewport.height) +
                      static_cast<float>(viewport.y);
  return 1.0f - (pixel / static_cast<float>(kHeight)) * 2.0f;
}

float ShadeNormal(Vec3 normal) {
  // Key light from upper-right; higher ambient so stills are not muddy.
  const Vec3 light = Normalize({0.55f, -0.25f, 0.80f});
  float lambert = Dot(normal, light);
  if (lambert < 0.0f) {
    lambert *= -0.25f;
  }
  return std::clamp(0.48f + 0.55f * lambert, 0.40f, 1.0f);
}

float FaceShade(Vec3 a, Vec3 b, Vec3 c) {
  return ShadeNormal(Normalize(Cross(Sub(b, a), Sub(c, a))));
}

float OriginShade(Vec3 point) {
  return ShadeNormal(Normalize(point));
}

void VertexColor(const PresentMesh& mesh, std::uint32_t index, float rgba[4]) {
  const size_t base = static_cast<size_t>(index) * 4;
  if (base + 3 < mesh.colors.size()) {
    rgba[0] = mesh.colors[base];
    rgba[1] = mesh.colors[base + 1];
    rgba[2] = mesh.colors[base + 2];
    rgba[3] = mesh.colors[base + 3];
    return;
  }
  rgba[0] = mesh.color[0];
  rgba[1] = mesh.color[1];
  rgba[2] = mesh.color[2];
  rgba[3] = mesh.color[3];
}

void PutPixel(std::vector<std::uint8_t>* image, int x, int y, const float color[4]) {
  if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) {
    return;
  }
  std::uint8_t* pixel = image->data() + (static_cast<size_t>(y) * kWidth + x) * 4;
  pixel[0] = static_cast<std::uint8_t>(std::clamp(color[2], 0.0f, 1.0f) * 255.0f);
  pixel[1] = static_cast<std::uint8_t>(std::clamp(color[1], 0.0f, 1.0f) * 255.0f);
  pixel[2] = static_cast<std::uint8_t>(std::clamp(color[0], 0.0f, 1.0f) * 255.0f);
  pixel[3] = 255;
}

void FillTriangle(std::vector<std::uint8_t>* image,
                  std::vector<float>* depth,
                  std::uint32_t* drawn,
                  NdcVertex a,
                  NdcVertex b,
                  NdcVertex c,
                  const Viewport& viewport,
                  bool underlay) {
  if (!a.visible || !b.visible || !c.visible) {
    return;
  }
  const auto to_x = [&viewport](float ndc) {
    return (ndc * 0.5f + 0.5f) * static_cast<float>(viewport.width) +
           static_cast<float>(viewport.x);
  };
  const auto to_y = [&viewport](float ndc) {
    return (1.0f - (ndc * 0.5f + 0.5f)) * static_cast<float>(viewport.height) +
           static_cast<float>(viewport.y);
  };
  const float ax = to_x(a.x);
  const float ay = to_y(a.y);
  const float bx = to_x(b.x);
  const float by = to_y(b.y);
  const float cx = to_x(c.x);
  const float cy = to_y(c.y);
  const float area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
  if (std::fabs(area) < 1.0f) {
    return;
  }
  // A tile that crosses the frustum still covers the viewport. Dropping the
  // whole triangle because one vertex is outside NDC leaves the PNG empty.
  const int min_x = std::max(viewport.x, static_cast<int>(std::floor(std::min({ax, bx, cx}))));
  const int max_x = std::min(viewport.x + viewport.width - 1,
                             static_cast<int>(std::ceil(std::max({ax, bx, cx}))));
  const int min_y = std::max(viewport.y, static_cast<int>(std::floor(std::min({ay, by, cy}))));
  const int max_y = std::min(viewport.y + viewport.height - 1,
                             static_cast<int>(std::ceil(std::max({ay, by, cy}))));
  for (int y = min_y; y <= max_y; ++y) {
    for (int x = min_x; x <= max_x; ++x) {
      const float px = static_cast<float>(x) + 0.5f;
      const float py = static_cast<float>(y) + 0.5f;
      const float w0 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
      const float w1 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
      const float w2 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);
      const bool inside = (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
      if (!inside) {
        continue;
      }
      // w1/w2/w0 are screen-space areas for vertices A/B/C. Interpolate 1/z so a
      // coarse tile cannot paint a nearer depth over the city skirt.
      const float weight_sum = w0 + w1 + w2;
      const float inv_z_sum = w1 / a.depth + w2 / b.depth + w0 / c.depth;
      if (weight_sum == 0.0f || inv_z_sum == 0.0f) {
        continue;
      }
      const float pixel_depth = weight_sum / inv_z_sum;
      std::uint8_t& alpha =
          (*image)[(static_cast<size_t>(y) * kWidth + x) * 4 + 3];
      if (underlay) {
        // Atmosphere / glow: paint only empty pixels; leave depth open for Earth.
        if (alpha != 0) {
          continue;
        }
      } else {
        float& stored = (*depth)[static_cast<size_t>(y) * kWidth + x];
        if (pixel_depth >= stored) {
          continue;
        }
        stored = pixel_depth;
      }
      if (alpha == 0) {
        ++(*drawn);
      }
      const float wa = (w1 / a.depth) / inv_z_sum;
      const float wb = (w2 / b.depth) / inv_z_sum;
      const float wc = (w0 / c.depth) / inv_z_sum;
      const float shade_r = a.r * wa + b.r * wb + c.r * wc;
      const float shade_g = a.g * wa + b.g * wb + c.g * wc;
      const float shade_b = a.b * wa + b.b * wb + c.b * wc;
      const float shaded[4] = {shade_r, shade_g, shade_b, 1.0f};
      PutPixel(image, x, y, shaded);
    }
  }
}

void DrawLine(std::vector<std::uint8_t>* image, float x0, float y0, float x1, float y1) {
  const float edge[4] = {0.95f, 0.95f, 0.95f, 1.0f};
  int ix = static_cast<int>(std::lround(x0));
  int iy = static_cast<int>(std::lround(y0));
  const int tx = static_cast<int>(std::lround(x1));
  const int ty = static_cast<int>(std::lround(y1));
  const int dx = std::abs(tx - ix);
  const int dy = -std::abs(ty - iy);
  const int sx = ix < tx ? 1 : -1;
  const int sy = iy < ty ? 1 : -1;
  int err = dx + dy;
  for (int step = 0; step < 4000; ++step) {
    PutPixel(image, ix, iy, edge);
    if (ix == tx && iy == ty) {
      return;
    }
    const int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      ix += sx;
    }
    if (e2 <= dx) {
      err += dx;
      iy += sy;
    }
  }
}

void PaintBackground(std::vector<std::uint8_t>* image) {
  for (int y = 0; y < kHeight; ++y) {
    const float t = static_cast<float>(y) / static_cast<float>(kHeight - 1);
    // Space-like falloff, kept a bit brighter so the globe does not sink into ink.
    const float background[4] = {0.04f + 0.12f * t, 0.06f + 0.16f * t,
                                 0.10f + 0.26f * t, 1.0f};
    for (int x = 0; x < kWidth; ++x) {
      if ((*image)[(static_cast<size_t>(y) * kWidth + x) * 4 + 3] == 0) {
        PutPixel(image, x, y, background);
      }
    }
  }
}

std::uint32_t DrawMeshes(const PresentCamera& camera,
                         const std::vector<PresentMesh>& meshes,
                         std::vector<std::uint8_t>* image,
                         std::vector<float>* depth,
                         std::vector<float>* gpu_vertices,
                         std::vector<std::uint32_t>* indices,
                         const Viewport& viewport) {
  std::vector<float> edges;
  std::uint32_t drawn = 0;
  for (const PresentMesh& mesh : meshes) {
    const std::uint32_t vertex_count =
        static_cast<std::uint32_t>(mesh.mesh.xyz.size() / 3);
    std::vector<NdcVertex> projected(vertex_count);
    std::vector<Vec3> world(vertex_count);
    for (std::uint32_t vertex = 0; vertex < vertex_count; ++vertex) {
      const size_t offset = static_cast<size_t>(vertex) * 3;
      world[vertex] = {mesh.mesh.xyz[offset], mesh.mesh.xyz[offset + 1],
                       mesh.mesh.xyz[offset + 2]};
      projected[vertex] = Project(camera, world[vertex], viewport.width, viewport.height);
      float rgba[4];
      VertexColor(mesh, vertex, rgba);
      projected[vertex].r = rgba[0];
      projected[vertex].g = rgba[1];
      projected[vertex].b = rgba[2];
    }
    for (size_t i = 0; i + 2 < mesh.mesh.indices.size(); i += 3) {
      const std::uint32_t i0 = mesh.mesh.indices[i];
      const std::uint32_t i1 = mesh.mesh.indices[i + 1];
      const std::uint32_t i2 = mesh.mesh.indices[i + 2];
      if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count) {
        continue;
      }
      NdcVertex va = projected[i0];
      NdcVertex vb = projected[i1];
      NdcVertex vc = projected[i2];
      if (!camera.skip_shade) {
        if (camera.shade_from_origin) {
          const float sa = OriginShade(world[i0]);
          const float sb = OriginShade(world[i1]);
          const float sc = OriginShade(world[i2]);
          va.r *= sa;
          va.g *= sa;
          va.b *= sa;
          vb.r *= sb;
          vb.g *= sb;
          vb.b *= sb;
          vc.r *= sc;
          vc.g *= sc;
          vc.b *= sc;
        } else {
          const float shade = FaceShade(world[i0], world[i1], world[i2]);
          va.r *= shade;
          va.g *= shade;
          va.b *= shade;
          vb.r *= shade;
          vb.g *= shade;
          vb.b *= shade;
          vc.r *= shade;
          vc.g *= shade;
          vc.b *= shade;
        }
      }
      FillTriangle(image, depth, &drawn, va, vb, vc, viewport, mesh.underlay);
      if (camera.draw_edges && va.visible && vb.visible && vc.visible) {
        // Ground planes project to huge NDC; stroking them paints white
        // diagonals across the whole frame (including the other split panel).
        const auto in_panel = [](const NdcVertex& v) {
          return std::fabs(v.x) <= 1.05f && std::fabs(v.y) <= 1.05f;
        };
        if (in_panel(va) && in_panel(vb) && in_panel(vc)) {
          const auto to_x = [&viewport](float ndc) {
            return (ndc * 0.5f + 0.5f) * static_cast<float>(viewport.width) +
                   static_cast<float>(viewport.x);
          };
          const auto to_y = [&viewport](float ndc) {
            return (1.0f - (ndc * 0.5f + 0.5f)) * static_cast<float>(viewport.height) +
                   static_cast<float>(viewport.y);
          };
          edges.insert(edges.end(),
                       {to_x(va.x), to_y(va.y), to_x(vb.x), to_y(vb.y),
                        to_x(vb.x), to_y(vb.y), to_x(vc.x), to_y(vc.y),
                        to_x(vc.x), to_y(vc.y), to_x(va.x), to_y(va.y)});
        }
      }
      // ECEF-scale globes project many back-facing / far triangles to huge NDC
      // values that crash the FlyCube upload path. Keep the CPU raster, skip GPU.
      const auto gpu_ok = [](const NdcVertex& v) {
        return v.visible && std::isfinite(v.x) && std::isfinite(v.y) &&
               std::fabs(v.x) < 4.0f && std::fabs(v.y) < 4.0f;
      };
      if (!gpu_ok(va) && !gpu_ok(vb) && !gpu_ok(vc)) {
        continue;
      }
      const std::uint32_t base =
          static_cast<std::uint32_t>(gpu_vertices->size() / 6);
      const NdcVertex corners[3] = {va, vb, vc};
      for (const NdcVertex& corner : corners) {
        const float nx = std::clamp(corner.x, -4.0f, 4.0f);
        const float ny = std::clamp(corner.y, -4.0f, 4.0f);
        gpu_vertices->insert(gpu_vertices->end(),
                             {ViewportNdcX(nx, viewport), ViewportNdcY(ny, viewport),
                              0.5f, corner.r, corner.g, corner.b});
      }
      indices->push_back(base);
      indices->push_back(base + 1);
      indices->push_back(base + 2);
    }
  }
  if (camera.draw_edges) {
    for (size_t i = 0; i + 3 < edges.size(); i += 4) {
      DrawLine(image, edges[i], edges[i + 1], edges[i + 2], edges[i + 3]);
    }
  }
  return drawn;
}

std::uint32_t Rasterize(const PresentCamera& camera,
                        const std::vector<PresentMesh>& meshes,
                        std::vector<std::uint8_t>* image,
                        std::vector<float>* gpu_vertices,
                        std::vector<std::uint32_t>* indices) {
  image->assign(static_cast<size_t>(kWidth) * kHeight * 4, 0);
  std::vector<float> depth(static_cast<size_t>(kWidth) * kHeight, 1.0e30f);
  const std::uint32_t drawn =
      DrawMeshes(camera, meshes, image, &depth, gpu_vertices, indices, Viewport{});
  PaintBackground(image);
  return drawn;
}

bool WritePng(const std::wstring& path, const std::vector<std::uint8_t>& image) {
  const HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool should_uninit = co == S_OK;
  IWICImagingFactory* factory = nullptr;
  if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&factory)))) {
    if (should_uninit) {
      CoUninitialize();
    }
    return false;
  }
  IWICStream* stream = nullptr;
  IWICBitmapEncoder* encoder = nullptr;
  IWICBitmapFrameEncode* frame = nullptr;
  bool ok = false;
  if (SUCCEEDED(factory->CreateStream(&stream)) &&
      SUCCEEDED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE)) &&
      SUCCEEDED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) &&
      SUCCEEDED(encoder->Initialize(stream, WICBitmapEncoderNoCache)) &&
      SUCCEEDED(encoder->CreateNewFrame(&frame, nullptr)) &&
      SUCCEEDED(frame->Initialize(nullptr))) {
    frame->SetSize(kWidth, kHeight);
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    if (SUCCEEDED(frame->SetPixelFormat(&format)) &&
        SUCCEEDED(frame->WritePixels(kHeight, kWidth * 4,
                                     static_cast<UINT>(image.size()),
                                     const_cast<BYTE*>(image.data()))) &&
        SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit())) {
      ok = true;
    }
  }
  if (frame) {
    frame->Release();
  }
  if (encoder) {
    encoder->Release();
  }
  if (stream) {
    stream->Release();
  }
  factory->Release();
  if (should_uninit) {
    CoUninitialize();
  }
  return ok;
}

bool EnsureWindowClass(const wchar_t* class_name,
                       WNDPROC proc,
                       HINSTANCE instance_handle) {
  WNDCLASSW existing = {};
  if (GetClassInfoW(instance_handle, class_name, &existing)) {
    return true;
  }
  WNDCLASSW window_class = {};
  window_class.lpfnWndProc = proc;
  window_class.hInstance = instance_handle;
  window_class.lpszClassName = class_name;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  return RegisterClassW(&window_class) != 0;
}

// C++ body (may throw). Kept out of the SEH frame below.
bool CreateDx12DeviceInner(std::shared_ptr<Instance>* instance,
                           std::shared_ptr<Adapter>* adapter,
                           std::shared_ptr<Device>* device,
                           char* error,
                           size_t error_bytes) {
  *instance = CreateInstance(ApiType::kDX12);
  if (!*instance) {
    std::snprintf(error, error_bytes, "CreateInstance returned null");
    return false;
  }
  const auto adapters = (*instance)->EnumerateAdapters();
  if (adapters.empty()) {
    std::snprintf(error, error_bytes, "no DX12 adapters");
    return false;
  }
  *adapter = adapters[0];
  *device = (*adapter)->CreateDevice();
  if (!*device) {
    std::snprintf(error, error_bytes, "CreateDevice returned null");
    return false;
  }
  return true;
}

// SEH wrapper: CreateInstance can AV when DX12 is unusable in the session.
bool TryCreateDx12Device(std::shared_ptr<Instance>* instance,
                         std::shared_ptr<Adapter>* adapter,
                         std::shared_ptr<Device>* device,
                         std::string* error) {
  char message[128] = {};
  bool ok = false;
  __try {
    ok = CreateDx12DeviceInner(instance, adapter, device, message, sizeof(message));
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    std::snprintf(message, sizeof(message), "DX12 device init faulted");
    ok = false;
  }
  if (!ok) {
    *error = message[0] ? message : "DX12 device init failed";
  }
  return ok;
}

void ShowOnFlyCube(const char* title,
                   const std::vector<float>& gpu_vertices,
                   const std::vector<std::uint32_t>& indices) {
  if (gpu_vertices.empty() || indices.empty()) {
    throw std::runtime_error("ShowOnFlyCube needs non-empty mesh");
  }
  const HINSTANCE instance_handle = GetModuleHandleW(nullptr);
  const wchar_t* class_name = L"CesiumFlyCubePresent";
  if (!EnsureWindowClass(class_name, PresentWndProc, instance_handle)) {
    throw std::runtime_error("RegisterClassW failed");
  }

  RECT rect{0, 0, kWidth, kHeight};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = CreateWindowExW(0, class_name, L"Cesium for FlyCube", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                              nullptr, nullptr, instance_handle, nullptr);
  if (!hwnd) {
    throw std::runtime_error("CreateWindowEx failed");
  }
  SetWindowTextA(hwnd, title);

  std::shared_ptr<Instance> instance;
  std::shared_ptr<Adapter> adapter;
  std::shared_ptr<Device> device;
  std::string gpu_error;
  if (!TryCreateDx12Device(&instance, &adapter, &device, &gpu_error)) {
    DestroyWindow(hwnd);
    throw std::runtime_error(gpu_error);
  }
  std::shared_ptr<CommandQueue> queue = device->GetCommandQueue(CommandListType::kGraphics);
  Win32Surface win32{instance_handle, hwnd};
  constexpr uint32_t kFrameCount = 3;
  std::shared_ptr<Swapchain> swapchain =
      device->CreateSwapchain(win32, kWidth, kHeight, kFrameCount, false);
  std::shared_ptr<Fence> fence = device->CreateFence(0);
  uint64_t fence_value = 0;

  std::shared_ptr<Resource> vertex_buffer = device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{gpu_vertices.size() * sizeof(float), BindFlag::kVertexBuffer});
  vertex_buffer->UpdateUploadBuffer(0, gpu_vertices.data(), gpu_vertices.size() * sizeof(float));
  std::shared_ptr<Resource> index_buffer = device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{indices.size() * sizeof(std::uint32_t), BindFlag::kIndexBuffer});
  index_buffer->UpdateUploadBuffer(0, indices.data(), indices.size() * sizeof(std::uint32_t));

  BindingSetLayoutDesc layout_desc;
  std::shared_ptr<BindingSetLayout> layout = device->CreateBindingSetLayout(layout_desc);

  const std::string shader_dir = CESIUM_FLYCUBE_SHADER_DIR;
  std::shared_ptr<Shader> vertex_shader = device->CompileShader(
      {shader_dir + "/SceneVs.hlsl", "main", ShaderType::kVertex, "6_0"});
  std::shared_ptr<Shader> pixel_shader = device->CompileShader(
      {shader_dir + "/ScenePs.hlsl", "main", ShaderType::kPixel, "6_0"});
  GraphicsPipelineDesc pipeline_desc;
  pipeline_desc.shaders = {vertex_shader, pixel_shader};
  pipeline_desc.layout = layout;
  pipeline_desc.input = {
      {0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6, 0},
      {0, "COLOR", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6, sizeof(float) * 3},
  };
  pipeline_desc.color_formats = {swapchain->GetFormat()};
  pipeline_desc.depth_stencil_desc.depth_test_enable = false;
  pipeline_desc.depth_stencil_desc.depth_write_enable = false;
  std::shared_ptr<Pipeline> pipeline = device->CreateGraphicsPipeline(pipeline_desc);

  std::array<std::shared_ptr<CommandList>, kFrameCount> command_lists;
  std::array<uint64_t, kFrameCount> frame_fences{};
  for (uint32_t frame = 0; frame < kFrameCount; ++frame) {
    std::shared_ptr<Resource> back_buffer = swapchain->GetBackBuffer(frame);
    ViewDesc target_desc{ViewType::kRenderTarget, ViewDimension::kTexture2D};
    std::shared_ptr<View> target = device->CreateView(back_buffer, target_desc);
    std::shared_ptr<CommandList> commands = device->CreateCommandList(CommandListType::kGraphics);
    commands->ResourceBarrier({{back_buffer, ResourceState::kPresent, ResourceState::kRenderTarget}});
    RenderPassDesc pass;
    pass.render_area = {0, 0, kWidth, kHeight};
    pass.colors.push_back(RenderPassColorDesc{
        target, RenderPassLoadOp::kClear, RenderPassStoreOp::kStore, {0.07f, 0.12f, 0.18f, 1.0f}});
    commands->BeginRenderPass(pass);
    commands->BindPipeline(pipeline);
    commands->SetViewport(0, 0, kWidth, kHeight, 0.0f, 1.0f);
    commands->SetScissorRect(0, 0, kWidth, kHeight);
    commands->IASetIndexBuffer(index_buffer, 0, gli::format::FORMAT_R32_UINT_PACK32);
    commands->IASetVertexBuffer(0, vertex_buffer, 0);
    commands->DrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    commands->EndRenderPass();
    commands->ResourceBarrier({{back_buffer, ResourceState::kRenderTarget, ResourceState::kPresent}});
    commands->Close();
    command_lists[frame] = commands;
  }

  for (int frame = 0; frame < kFrames; ++frame) {
    MSG message;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    const uint32_t image = swapchain->NextImage(fence, ++fence_value);
    queue->Wait(fence, fence_value);
    fence->Wait(frame_fences[image]);
    queue->ExecuteCommandLists({command_lists[image]});
    queue->Signal(fence, frame_fences[image] = ++fence_value);
    swapchain->Present(fence, frame_fences[image]);
    Sleep(16);
  }
  queue->Signal(fence, ++fence_value);
  fence->Wait(fence_value);
  DestroyWindow(hwnd);
}

struct BrowseInput {
  bool left = false;
  bool right = false;
  int last_x = 0;
  int last_y = 0;
  bool tracking = false;
  double yaw_pixels = 0;
  double pitch_pixels = 0;
  double pan_x = 0;
  double pan_y = 0;
  double zoom_steps = 0;
  bool closed = false;
};

LRESULT CALLBACK BrowseWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_NCCREATE) {
    const auto* created = reinterpret_cast<CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created->lpCreateParams));
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }
  auto* input = reinterpret_cast<BrowseInput*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!input) {
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }
  switch (message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      input->left = message == WM_LBUTTONDOWN || input->left;
      input->right = message == WM_RBUTTONDOWN || input->right;
      input->last_x = GET_X_LPARAM(lparam);
      input->last_y = GET_Y_LPARAM(lparam);
      input->tracking = true;
      SetCapture(hwnd);
      return 0;
    case WM_LBUTTONUP:
      input->left = false;
      if (!input->right) {
        input->tracking = false;
        ReleaseCapture();
      }
      return 0;
    case WM_RBUTTONUP:
      input->right = false;
      if (!input->left) {
        input->tracking = false;
        ReleaseCapture();
      }
      return 0;
    case WM_MOUSEMOVE:
      if (input->tracking) {
        const int x = GET_X_LPARAM(lparam);
        const int y = GET_Y_LPARAM(lparam);
        const double dx = static_cast<double>(x - input->last_x);
        const double dy = static_cast<double>(y - input->last_y);
        input->last_x = x;
        input->last_y = y;
        if (input->left) {
          input->yaw_pixels += dx;
          input->pitch_pixels += dy;
        }
        if (input->right) {
          input->pan_x += dx;
          input->pan_y += dy;
        }
      }
      return 0;
    case WM_MOUSEWHEEL:
      input->zoom_steps += static_cast<double>(GET_WHEEL_DELTA_WPARAM(wparam)) / WHEEL_DELTA;
      return 0;
    case WM_KEYDOWN:
      if (wparam == VK_ESCAPE) {
        input->closed = true;
        DestroyWindow(hwnd);
        return 0;
      }
      break;
    case WM_CLOSE:
      input->closed = true;
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      input->closed = true;
      return 0;
    default:
      break;
  }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}

template <typename Tick>
void RunBrowseWindow(const char* title,
                     BrowseInput* input,
                     std::vector<float> gpu_vertices,
                     const std::vector<std::uint32_t>& indices,
                     Tick tick) {
  if (gpu_vertices.empty() || indices.empty()) {
    throw std::runtime_error("browse needs non-empty mesh");
  }
  const HINSTANCE instance_handle = GetModuleHandleW(nullptr);
  const wchar_t* class_name = L"CesiumFlyCubeBrowse";
  if (!EnsureWindowClass(class_name, BrowseWndProc, instance_handle)) {
    throw std::runtime_error("RegisterClassW failed");
  }

  RECT rect{0, 0, kWidth, kHeight};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = CreateWindowExW(0, class_name, L"Cesium for FlyCube",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                              rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
                              instance_handle, input);
  if (!hwnd) {
    throw std::runtime_error("CreateWindowEx failed");
  }
  SetWindowTextA(hwnd, title);

  std::shared_ptr<Instance> instance;
  std::shared_ptr<Adapter> adapter;
  std::shared_ptr<Device> device;
  std::string gpu_error;
  if (!TryCreateDx12Device(&instance, &adapter, &device, &gpu_error)) {
    DestroyWindow(hwnd);
    throw std::runtime_error(gpu_error);
  }
  std::shared_ptr<CommandQueue> queue = device->GetCommandQueue(CommandListType::kGraphics);
  Win32Surface win32{instance_handle, hwnd};
  constexpr uint32_t kFrameCount = 3;
  std::shared_ptr<Swapchain> swapchain =
      device->CreateSwapchain(win32, kWidth, kHeight, kFrameCount, false);
  std::shared_ptr<Fence> fence = device->CreateFence(0);
  uint64_t fence_value = 0;

  std::shared_ptr<Resource> vertex_buffer = device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{gpu_vertices.size() * sizeof(float), BindFlag::kVertexBuffer});
  vertex_buffer->UpdateUploadBuffer(0, gpu_vertices.data(), gpu_vertices.size() * sizeof(float));
  std::shared_ptr<Resource> index_buffer = device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{indices.size() * sizeof(std::uint32_t), BindFlag::kIndexBuffer});
  index_buffer->UpdateUploadBuffer(0, indices.data(), indices.size() * sizeof(std::uint32_t));

  BindingSetLayoutDesc layout_desc;
  std::shared_ptr<BindingSetLayout> layout = device->CreateBindingSetLayout(layout_desc);
  const std::string shader_dir = CESIUM_FLYCUBE_SHADER_DIR;
  std::shared_ptr<Shader> vertex_shader = device->CompileShader(
      {shader_dir + "/SceneVs.hlsl", "main", ShaderType::kVertex, "6_0"});
  std::shared_ptr<Shader> pixel_shader = device->CompileShader(
      {shader_dir + "/ScenePs.hlsl", "main", ShaderType::kPixel, "6_0"});
  GraphicsPipelineDesc pipeline_desc;
  pipeline_desc.shaders = {vertex_shader, pixel_shader};
  pipeline_desc.layout = layout;
  pipeline_desc.input = {
      {0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6, 0},
      {0, "COLOR", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6, sizeof(float) * 3},
  };
  pipeline_desc.color_formats = {swapchain->GetFormat()};
  pipeline_desc.depth_stencil_desc.depth_test_enable = false;
  pipeline_desc.depth_stencil_desc.depth_write_enable = false;
  std::shared_ptr<Pipeline> pipeline = device->CreateGraphicsPipeline(pipeline_desc);

  std::array<std::shared_ptr<CommandList>, kFrameCount> command_lists;
  std::array<uint64_t, kFrameCount> frame_fences{};
  for (uint32_t frame = 0; frame < kFrameCount; ++frame) {
    std::shared_ptr<Resource> back_buffer = swapchain->GetBackBuffer(frame);
    ViewDesc target_desc{ViewType::kRenderTarget, ViewDimension::kTexture2D};
    std::shared_ptr<View> target = device->CreateView(back_buffer, target_desc);
    std::shared_ptr<CommandList> commands = device->CreateCommandList(CommandListType::kGraphics);
    commands->ResourceBarrier({{back_buffer, ResourceState::kPresent, ResourceState::kRenderTarget}});
    RenderPassDesc pass;
    pass.render_area = {0, 0, kWidth, kHeight};
    pass.colors.push_back(RenderPassColorDesc{
        target, RenderPassLoadOp::kClear, RenderPassStoreOp::kStore, {0.07f, 0.12f, 0.18f, 1.0f}});
    commands->BeginRenderPass(pass);
    commands->BindPipeline(pipeline);
    commands->SetViewport(0, 0, kWidth, kHeight, 0.0f, 1.0f);
    commands->SetScissorRect(0, 0, kWidth, kHeight);
    commands->IASetIndexBuffer(index_buffer, 0, gli::format::FORMAT_R32_UINT_PACK32);
    commands->IASetVertexBuffer(0, vertex_buffer, 0);
    commands->DrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    commands->EndRenderPass();
    commands->ResourceBarrier({{back_buffer, ResourceState::kRenderTarget, ResourceState::kPresent}});
    commands->Close();
    command_lists[frame] = commands;
  }

  const size_t vertex_bytes = gpu_vertices.size() * sizeof(float);
  while (!input->closed) {
    MSG message;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    if (input->closed) {
      break;
    }
    if (!tick(&gpu_vertices)) {
      break;
    }
    if (gpu_vertices.size() * sizeof(float) != vertex_bytes) {
      throw std::runtime_error("browse frame changed vertex count");
    }
    for (uint64_t waited : frame_fences) {
      fence->Wait(waited);
    }
    vertex_buffer->UpdateUploadBuffer(0, gpu_vertices.data(), vertex_bytes);
    const uint32_t image = swapchain->NextImage(fence, ++fence_value);
    queue->Wait(fence, fence_value);
    fence->Wait(frame_fences[image]);
    queue->ExecuteCommandLists({command_lists[image]});
    queue->Signal(fence, frame_fences[image] = ++fence_value);
    swapchain->Present(fence, frame_fences[image]);
    Sleep(16);
  }
  queue->Signal(fence, ++fence_value);
  fence->Wait(fence_value);
  if (IsWindow(hwnd)) {
    DestroyWindow(hwnd);
  }
}

void StampHudOnBgra(std::vector<std::uint8_t>* image, const char* text) {
  if (!image || !text || !text[0] ||
      image->size() != static_cast<size_t>(kWidth) * kHeight * 4) {
    return;
  }
  BITMAPINFO info = {};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = kWidth;
  info.bmiHeader.biHeight = -kHeight;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  HDC hdc = CreateCompatibleDC(nullptr);
  if (!hdc) {
    return;
  }
  HBITMAP bmp = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  if (!bmp || !bits) {
    DeleteDC(hdc);
    return;
  }
  std::memcpy(bits, image->data(), image->size());
  const HGDIOBJ old_bmp = SelectObject(hdc, bmp);
  HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                           DEFAULT_PITCH | FF_SWISS, "Consolas");
  const HGDIOBJ old_font = font ? SelectObject(hdc, font) : nullptr;
  SetBkMode(hdc, OPAQUE);
  SetBkColor(hdc, RGB(8, 12, 20));
  SetTextColor(hdc, RGB(255, 230, 80));
  TextOutA(hdc, 10, 8, text, static_cast<int>(std::strlen(text)));
  // GDI leaves alpha=0 on 32bpp DIBs; PNG is BGRA so force opaque or the HUD vanishes.
  auto* pixels = static_cast<std::uint8_t*>(bits);
  for (int y = 0; y < 36; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      pixels[(static_cast<size_t>(y) * kWidth + x) * 4 + 3] = 255;
    }
  }
  std::memcpy(image->data(), bits, image->size());
  if (old_font) {
    SelectObject(hdc, old_font);
  }
  if (font) {
    DeleteObject(font);
  }
  SelectObject(hdc, old_bmp);
  DeleteObject(bmp);
  DeleteDC(hdc);
}

PresentCamera CameraFromPose(const CameraPose& pose, const PresentCamera& style) {
  PresentCamera camera = style;
  camera.eye[0] = static_cast<float>(pose.eye[0]);
  camera.eye[1] = static_cast<float>(pose.eye[1]);
  camera.eye[2] = static_cast<float>(pose.eye[2]);
  camera.target[0] = static_cast<float>(pose.target[0]);
  camera.target[1] = static_cast<float>(pose.target[1]);
  camera.target[2] = static_cast<float>(pose.target[2]);
  camera.up[0] = static_cast<float>(pose.up[0]);
  camera.up[1] = static_cast<float>(pose.up[1]);
  camera.up[2] = static_cast<float>(pose.up[2]);
  return camera;
}

}  // namespace

PresentResult PresentScene(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const std::string& png_path,
                           bool open_window,
                           bool stamp_fps) {
  PresentResult result;
  result.png_path = png_path;
  try {
    std::vector<std::uint8_t> image;
    std::vector<float> gpu_vertices;
    std::vector<std::uint32_t> indices;
    const auto raster_start = std::chrono::steady_clock::now();
    result.drawn_pixels = Rasterize(camera, meshes, &image, &gpu_vertices, &indices);
    const auto raster_end = std::chrono::steady_clock::now();
    result.frame_ms = static_cast<float>(
        std::chrono::duration<double, std::milli>(raster_end - raster_start).count());
    result.fps =
        result.frame_ms > 0.05f ? 1000.0f / result.frame_ms : 0.0f;
    if (stamp_fps) {
      char hud[96];
      std::snprintf(hud, sizeof(hud), "FPS %.1f  ms %.1f  px %u",
                    static_cast<double>(result.fps),
                    static_cast<double>(result.frame_ms), result.drawn_pixels);
      StampHudOnBgra(&image, hud);
    }
    std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());
    const std::wstring wide(png_path.begin(), png_path.end());
    if (!WritePng(wide, image)) {
      result.message = "png write failed";
      return result;
    }
    if (result.drawn_pixels < 200) {
      result.message = "scene projected to too few pixels";
      return result;
    }
    if (open_window) {
      if (gpu_vertices.empty() || indices.empty()) {
        result.message = "scene produced no GPU vertices";
        return result;
      }
      ShowOnFlyCube(title, gpu_vertices, indices);
    }
    result.ok = true;
    result.message = "presented";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

namespace {


CameraPose PoseFromCamera(const PresentCamera& camera) {
  CameraPose pose;
  pose.eye[0] = camera.eye[0];
  pose.eye[1] = camera.eye[1];
  pose.eye[2] = camera.eye[2];
  pose.target[0] = camera.target[0];
  pose.target[1] = camera.target[1];
  pose.target[2] = camera.target[2];
  pose.up[0] = camera.up[0];
  pose.up[1] = camera.up[1];
  pose.up[2] = camera.up[2];
  return pose;
}

}  // namespace

std::uint32_t CountDrawnPixels(const PresentCamera& camera,
                               const std::vector<PresentMesh>& meshes) {
  std::vector<std::uint8_t> image;
  std::vector<float> vertices;
  std::vector<std::uint32_t> indices;
  return Rasterize(camera, meshes, &image, &vertices, &indices);
}

void BlitBgra(HWND hwnd, const std::vector<std::uint8_t>& bgra, const char* hud) {
  BITMAPINFO info = {};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = kWidth;
  info.bmiHeader.biHeight = -kHeight;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  HDC hdc = GetDC(hwnd);
  if (!hdc) {
    return;
  }
  StretchDIBits(hdc, 0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, bgra.data(), &info,
                DIB_RGB_COLORS, SRCCOPY);
  if (hud && hud[0]) {
    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Consolas");
    const HGDIOBJ old_font = font ? SelectObject(hdc, font) : nullptr;
    SetBkMode(hdc, OPAQUE);
    SetBkColor(hdc, RGB(8, 12, 20));
    SetTextColor(hdc, RGB(255, 230, 80));
    TextOutA(hdc, 10, 8, hud, static_cast<int>(std::strlen(hud)));
    if (old_font) {
      SelectObject(hdc, old_font);
    }
    if (font) {
      DeleteObject(font);
    }
  }
  ReleaseDC(hwnd, hdc);
}

// CPU raster + GDI present. Avoids FlyCube DX12 AVs in headless / bad-adapter sessions.
// `tick` returns false to stop; optional `hud` is drawn each frame (FPS, etc.).
void RunBrowseGdi(const char* title,
                  BrowseInput* input,
                  const std::function<bool(std::vector<std::uint8_t>*, std::string*)>& tick) {
  const HINSTANCE instance_handle = GetModuleHandleW(nullptr);
  const wchar_t* class_name = L"CesiumFlyCubeBrowseGdi";
  if (!EnsureWindowClass(class_name, BrowseWndProc, instance_handle)) {
    throw std::runtime_error("RegisterClassW failed");
  }
  RECT rect{0, 0, kWidth, kHeight};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = CreateWindowExW(0, class_name, L"Cesium for FlyCube",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                              rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
                              instance_handle, input);
  if (!hwnd) {
    throw std::runtime_error("CreateWindowEx failed");
  }
  SetWindowTextA(hwnd, title);
  std::vector<std::uint8_t> frame;
  std::string hud;
  while (!input->closed) {
    MSG message;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    if (input->closed) {
      break;
    }
    if (!tick(&frame, &hud)) {
      break;
    }
    if (frame.size() == static_cast<size_t>(kWidth) * kHeight * 4) {
      BlitBgra(hwnd, frame, hud.c_str());
    }
    if (!hud.empty()) {
      char window_title[256];
      std::snprintf(window_title, sizeof(window_title), "%s  |  %s", title, hud.c_str());
      SetWindowTextA(hwnd, window_title);
    }
    // No Sleep: FPS reflects CPU raster cost, not an artificial 60 Hz cap.
  }
  if (IsWindow(hwnd)) {
    DestroyWindow(hwnd);
  }
}

// Expanded triangle soup in world space so DX12 uploads keep a fixed vertex count.
struct GpuTriCache {
  std::vector<Vec3> world;
  std::vector<float> rgb;
  std::vector<std::uint32_t> indices;
};

void GpuTriCacheFromProduct(const ProductMeshCache& product, GpuTriCache* cache) {
  cache->world.clear();
  cache->rgb = product.rgb;
  cache->indices = product.indices;
  cache->world.reserve(product.world_xyz.size() / 3);
  for (size_t i = 0; i + 2 < product.world_xyz.size(); i += 3) {
    cache->world.push_back(
        {product.world_xyz[i], product.world_xyz[i + 1], product.world_xyz[i + 2]});
  }
}

bool BuildProductMeshCache(const PresentCamera& style,
                           const std::vector<PresentMesh>& meshes,
                           ProductMeshCache* cache) {
  if (!cache)
    return false;
  cache->world_xyz.clear();
  cache->rgb.clear();
  cache->indices.clear();
  for (const PresentMesh& mesh : meshes) {
    const std::uint32_t vertex_count =
        static_cast<std::uint32_t>(mesh.mesh.xyz.size() / 3);
    for (size_t i = 0; i + 2 < mesh.mesh.indices.size(); i += 3) {
      const std::uint32_t idx[3] = {mesh.mesh.indices[i], mesh.mesh.indices[i + 1],
                                    mesh.mesh.indices[i + 2]};
      if (idx[0] >= vertex_count || idx[1] >= vertex_count ||
          idx[2] >= vertex_count) {
        continue;
      }
      Vec3 world[3];
      float rgb[3][3];
      for (int c = 0; c < 3; ++c) {
        const size_t offset = static_cast<size_t>(idx[c]) * 3;
        world[c] = {mesh.mesh.xyz[offset], mesh.mesh.xyz[offset + 1],
                    mesh.mesh.xyz[offset + 2]};
        float rgba[4];
        VertexColor(mesh, idx[c], rgba);
        rgb[c][0] = rgba[0];
        rgb[c][1] = rgba[1];
        rgb[c][2] = rgba[2];
      }
      if (!style.skip_shade) {
        const float shade = FaceShade(world[0], world[1], world[2]);
        for (int c = 0; c < 3; ++c) {
          rgb[c][0] *= shade;
          rgb[c][1] *= shade;
          rgb[c][2] *= shade;
        }
      }
      const std::uint32_t base =
          static_cast<std::uint32_t>(cache->world_xyz.size() / 3);
      for (int c = 0; c < 3; ++c) {
        cache->world_xyz.push_back(world[c].x);
        cache->world_xyz.push_back(world[c].y);
        cache->world_xyz.push_back(world[c].z);
        cache->rgb.insert(cache->rgb.end(), {rgb[c][0], rgb[c][1], rgb[c][2]});
        cache->indices.push_back(base + static_cast<std::uint32_t>(c));
      }
    }
  }
  return !cache->indices.empty();
}

void BuildGpuTriCache(const PresentCamera& style,
                      const std::vector<PresentMesh>& meshes,
                      GpuTriCache* cache) {
  ProductMeshCache product;
  BuildProductMeshCache(style, meshes, &product);
  GpuTriCacheFromProduct(product, cache);
}

void ProjectGpuTriCache(const PresentCamera& camera,
                        const GpuTriCache& cache,
                        std::vector<float>* gpu_vertices) {
  gpu_vertices->clear();
  gpu_vertices->reserve(cache.world.size() * 6);
  const Viewport viewport{};
  for (size_t i = 0; i < cache.world.size(); ++i) {
    const NdcVertex ndc =
        Project(camera, cache.world[i], viewport.width, viewport.height);
    float x = ndc.visible ? ndc.x : 0.0f;
    float y = ndc.visible ? ndc.y : 0.0f;
    x = std::clamp(x, -2.0f, 2.0f);
    y = std::clamp(y, -2.0f, 2.0f);
    // Encode view depth so nearer triangles win when depth is enabled.
    const float z = ndc.visible ? std::clamp(ndc.depth / 5.0e7f, 0.001f, 0.999f) : 0.999f;
    gpu_vertices->push_back(ViewportNdcX(x, viewport));
    gpu_vertices->push_back(ViewportNdcY(y, viewport));
    gpu_vertices->push_back(z);
    gpu_vertices->push_back(cache.rgb[i * 3]);
    gpu_vertices->push_back(cache.rgb[i * 3 + 1]);
    gpu_vertices->push_back(cache.rgb[i * 3 + 2]);
  }
}

void ProjectProductMeshCache(const PresentCamera& camera,
                             const ProductMeshCache& cache,
                             std::vector<float>* gpu_vertices) {
  GpuTriCache tri;
  GpuTriCacheFromProduct(cache, &tri);
  ProjectGpuTriCache(camera, tri, gpu_vertices);
}

BrowseResult PresentBrowse(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const BrowseOptions& options) {
  BrowseResult result;
  try {
    const CameraPose start = PoseFromCamera(camera);
    const CameraPose destination =
        options.auto_fly ? PoseFromCamera(options.fly_target) : start;
    Flight flight;
    // Longer default so space→ground reads like Google Earth, not a snap cut.
    const double fly_seconds =
        options.seconds > 0 ? std::max(0.4, options.seconds * 0.65) : 6.5;
    if (options.auto_fly) {
      BeginFlight(&flight, start, destination, fly_seconds);
    }
    OrbitController orbit = OrbitFromPose(start);
    BrowseInput input;
    const PresentCamera first = CameraFromPose(start, camera);
    GpuTriCache cache;
    BuildGpuTriCache(first, meshes, &cache);
    if (cache.indices.empty()) {
      result.message = "browse mesh produced no triangles";
      return result;
    }
    result.drawn_pixels = static_cast<std::uint32_t>(cache.indices.size());

    double fps_sum = 0;
    double last_frame_sec = 1.0 / 60.0;
    auto session_start = std::chrono::steady_clock::now();

    auto advance = [&](PresentCamera* frame_cam) {
      if (input.yaw_pixels != 0 || input.pitch_pixels != 0 || input.pan_x != 0 ||
          input.pan_y != 0 || input.zoom_steps != 0) {
        if (flight.active) {
          orbit = OrbitFromPose(SampleFlight(flight));
          flight.active = false;
        }
        DragOrbit(&orbit, input.yaw_pixels, input.pitch_pixels);
        PanOrbit(&orbit, input.pan_x, input.pan_y);
        ZoomOrbit(&orbit, input.zoom_steps);
        input.yaw_pixels = 0;
        input.pitch_pixels = 0;
        input.pan_x = 0;
        input.pan_y = 0;
        input.zoom_steps = 0;
      }
      CameraPose pose = PoseFromOrbit(orbit);
      if (flight.active) {
        flight.elapsed_seconds += last_frame_sec;
        pose = SampleFlight(flight);
        if (flight.elapsed_seconds >= flight.duration_seconds) {
          flight.active = false;
          orbit = OrbitFromPose(pose);
          result.flew = true;
        }
      }
      *frame_cam = CameraFromPose(pose, camera);
      result.eye[0] = frame_cam->eye[0];
      result.eye[1] = frame_cam->eye[1];
      result.eye[2] = frame_cam->eye[2];
    };

    auto note_fps = [&](double frame_ms, std::string* hud) {
      ++result.frames;
      last_frame_sec = frame_ms / 1000.0;
      const float fps =
          frame_ms > 0.05 ? static_cast<float>(1000.0 / frame_ms) : 0.0f;
      result.fps = fps;
      fps_sum += fps;
      result.avg_fps =
          result.frames > 0 ? static_cast<float>(fps_sum / result.frames) : 0.0f;
      if (options.show_fps && hud) {
        char line[128];
        std::snprintf(line, sizeof(line), "%s  FPS %.1f  avg %.1f  ms %.1f  tris %u",
                      result.backend.c_str(), static_cast<double>(fps),
                      static_cast<double>(result.avg_fps), frame_ms,
                      static_cast<unsigned>(cache.indices.size() / 3));
        *hud = line;
      }
    };

    auto time_up = [&](const std::chrono::steady_clock::time_point& now) {
      const double elapsed =
          std::chrono::duration<double>(now - session_start).count();
      if (options.seconds > 0 && elapsed >= options.seconds) {
        if (flight.active && flight.elapsed_seconds + 0.02 >= flight.duration_seconds) {
          result.flew = true;
        }
        return true;
      }
      return false;
    };

    bool used_gpu = false;
    if (options.prefer_gpu) {
      try {
        result.backend = "dx12";
        std::vector<float> gpu_vertices;
        ProjectGpuTriCache(first, cache, &gpu_vertices);
        RunBrowseWindow(title, &input, gpu_vertices, cache.indices,
                        [&](std::vector<float>* vertices) {
                          const auto frame_start = std::chrono::steady_clock::now();
                          PresentCamera frame_cam;
                          advance(&frame_cam);
                          ProjectGpuTriCache(frame_cam, cache, vertices);
                          const auto frame_end = std::chrono::steady_clock::now();
                          const double frame_ms = std::chrono::duration<double, std::milli>(
                                                     frame_end - frame_start)
                                                     .count();
                          std::string hud;
                          note_fps(frame_ms, options.show_fps ? &hud : nullptr);
                          if (!hud.empty()) {
                            char window_title[256];
                            std::snprintf(window_title, sizeof(window_title), "%s  |  %s",
                                          title, hud.c_str());
                            // Title is updated from the HWND inside RunBrowseWindow via
                            // SetWindowText on the browse class — best-effort below.
                            SetWindowTextA(GetActiveWindow(), window_title);
                          }
                          return !time_up(frame_end);
                        });
        used_gpu = result.frames > 0;
      } catch (const std::exception& error) {
        result.backend = "gdi";
        result.message = std::string("dx12 fallback: ") + error.what();
        result.frames = 0;
        fps_sum = 0;
        flight.elapsed_seconds = 0;
        if (options.auto_fly) {
          flight.active = true;
        }
        orbit = OrbitFromPose(start);
        session_start = std::chrono::steady_clock::now();
      }
    }

    if (!used_gpu) {
      result.backend = "gdi";
      RunBrowseGdi(title, &input, [&](std::vector<std::uint8_t>* frame_image, std::string* hud) {
        const auto frame_start = std::chrono::steady_clock::now();
        PresentCamera frame_cam;
        advance(&frame_cam);
        std::vector<float> projected;
        std::vector<std::uint32_t> frame_indices;
        result.drawn_pixels =
            Rasterize(frame_cam, meshes, frame_image, &projected, &frame_indices);
        const auto frame_end = std::chrono::steady_clock::now();
        const double frame_ms =
            std::chrono::duration<double, std::milli>(frame_end - frame_start).count();
        note_fps(frame_ms, hud);
        return !time_up(frame_end);
      });
    }

    result.ok = result.frames > 0;
    if (result.message.empty() || result.message.rfind("dx12 fallback:", 0) == 0) {
      const std::string prefix = result.message;
      result.message = result.ok ? (result.backend + " browsed") : "browse produced no frames";
      if (!prefix.empty() && result.ok) {
        result.message += " (" + prefix + ")";
      }
    }
    if (options.auto_fly && options.seconds > 0 && !result.flew) {
      result.ok = false;
      result.message = "flight did not finish before the window closed";
    }
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

PresentResult PresentSplitScene(const char* title,
                                 const PresentCamera& left,
                                 const PresentCamera& right,
                                 const std::vector<PresentMesh>& meshes,
                                 const std::string& png_path) {
  PresentResult result;
  result.png_path = png_path;
  try {
    std::vector<std::uint8_t> image(static_cast<size_t>(kWidth) * kHeight * 4, 0);
    std::vector<float> depth(static_cast<size_t>(kWidth) * kHeight, 1.0e30f);
    std::vector<float> gpu_vertices;
    std::vector<std::uint32_t> indices;
    // Two 16:9 panels inside the 960x540 frame, with a gap between them.
    constexpr int kPanelWidth = 464;
    constexpr int kPanelHeight = 261;
    const int panel_y = (kHeight - kPanelHeight) / 2;
    const Viewport left_view{8, panel_y, kPanelWidth, kPanelHeight};
    const Viewport right_view{kWidth - 8 - kPanelWidth, panel_y, kPanelWidth, kPanelHeight};
    result.drawn_pixels =
        DrawMeshes(left, meshes, &image, &depth, &gpu_vertices, &indices, left_view);
    result.drawn_pixels +=
        DrawMeshes(right, meshes, &image, &depth, &gpu_vertices, &indices, right_view);
    PaintBackground(&image);
    std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());
    const std::wstring wide(png_path.begin(), png_path.end());
    if (!WritePng(wide, image)) {
      result.message = "png write failed";
      return result;
    }
    if (result.drawn_pixels < 200) {
      result.message = "scene projected to too few pixels";
      return result;
    }
    // Split proof is the PNG. Opening FlyCube here AVs in some sessions
    // (same path as google_earth --present); do not gate capture success on it.
    (void)title;
    (void)gpu_vertices;
    (void)indices;
    result.ok = true;
    result.message = "presented";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

}  // namespace cesium_flycube
