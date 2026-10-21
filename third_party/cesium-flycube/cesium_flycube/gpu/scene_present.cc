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
#include <cmath>
#include <filesystem>
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

float FaceShade(Vec3 a, Vec3 b, Vec3 c) {
  const Vec3 normal = Normalize(Cross(Sub(b, a), Sub(c, a)));
  const Vec3 light = Normalize({0.45f, -0.35f, 0.82f});
  float lambert = Dot(normal, light);
  if (lambert < 0.0f) {
    lambert *= -0.35f;
  }
  return std::clamp(0.22f + 0.78f * lambert, 0.18f, 1.0f);
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
                  const Viewport& viewport) {
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
      float& stored = (*depth)[static_cast<size_t>(y) * kWidth + x];
      if (pixel_depth >= stored) {
        continue;
      }
      stored = pixel_depth;
      if ((*image)[(static_cast<size_t>(y) * kWidth + x) * 4 + 3] == 0) {
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
    const float background[4] = {0.05f + 0.22f * t, 0.10f + 0.26f * t,
                                 0.18f + 0.20f * t, 1.0f};
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
      FillTriangle(image, depth, &drawn, va, vb, vc, viewport);
      if (camera.draw_edges && va.visible && vb.visible && vc.visible) {
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
      const std::uint32_t base =
          static_cast<std::uint32_t>(gpu_vertices->size() / 6);
      const NdcVertex corners[3] = {va, vb, vc};
      for (const NdcVertex& corner : corners) {
        gpu_vertices->insert(gpu_vertices->end(),
                             {ViewportNdcX(corner.x, viewport), ViewportNdcY(corner.y, viewport),
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

void ShowOnFlyCube(const char* title,
                   const std::vector<float>& gpu_vertices,
                   const std::vector<std::uint32_t>& indices) {
  const HINSTANCE instance_handle = GetModuleHandleW(nullptr);
  const wchar_t* class_name = L"CesiumFlyCubePresent";
  WNDCLASSW window_class = {};
  window_class.lpfnWndProc = PresentWndProc;
  window_class.hInstance = instance_handle;
  window_class.lpszClassName = class_name;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassW(&window_class);

  RECT rect{0, 0, kWidth, kHeight};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = CreateWindowExW(0, class_name, L"Cesium for FlyCube", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                              nullptr, nullptr, instance_handle, nullptr);
  if (!hwnd) {
    throw std::runtime_error("CreateWindowEx failed");
  }
  SetWindowTextA(hwnd, title);

  std::shared_ptr<Instance> instance = CreateInstance(ApiType::kDX12);
  std::shared_ptr<Adapter> adapter = instance->EnumerateAdapters().at(0);
  std::shared_ptr<Device> device = adapter->CreateDevice();
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
  const HINSTANCE instance_handle = GetModuleHandleW(nullptr);
  const wchar_t* class_name = L"CesiumFlyCubeBrowse";
  WNDCLASSW window_class = {};
  window_class.lpfnWndProc = BrowseWndProc;
  window_class.hInstance = instance_handle;
  window_class.lpszClassName = class_name;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassW(&window_class);

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

  std::shared_ptr<Instance> instance = CreateInstance(ApiType::kDX12);
  std::shared_ptr<Adapter> adapter = instance->EnumerateAdapters().at(0);
  std::shared_ptr<Device> device = adapter->CreateDevice();
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

}  // namespace

PresentResult PresentScene(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const std::string& png_path) {
  PresentResult result;
  result.png_path = png_path;
  try {
    std::vector<std::uint8_t> image;
    std::vector<float> gpu_vertices;
    std::vector<std::uint32_t> indices;
    result.drawn_pixels = Rasterize(camera, meshes, &image, &gpu_vertices, &indices);
    std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());
    const std::wstring wide(png_path.begin(), png_path.end());
    if (!WritePng(wide, image)) {
      result.message = "png write failed";
      return result;
    }
    if (result.drawn_pixels < 200 || gpu_vertices.empty() || indices.empty()) {
      result.message = "scene projected to too few pixels";
      return result;
    }
    ShowOnFlyCube(title, gpu_vertices, indices);
    result.ok = true;
    result.message = "presented";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

namespace {

PresentCamera CameraFromPose(const CameraPose& pose) {
  PresentCamera camera;
  camera.eye[0] = static_cast<float>(pose.eye[0]);
  camera.eye[1] = static_cast<float>(pose.eye[1]);
  camera.eye[2] = static_cast<float>(pose.eye[2]);
  camera.target[0] = static_cast<float>(pose.target[0]);
  camera.target[1] = static_cast<float>(pose.target[1]);
  camera.target[2] = static_cast<float>(pose.target[2]);
  camera.up[0] = 0;
  camera.up[1] = 0;
  camera.up[2] = 1;
  camera.fov_y_radians = 0.70f;
  camera.draw_edges = false;
  return camera;
}

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
    const double fly_seconds =
        options.seconds > 0 ? std::max(0.4, options.seconds * 0.65) : 5.0;
    if (options.auto_fly) {
      BeginFlight(&flight, start, destination, fly_seconds);
    }
    OrbitController orbit = OrbitFromPose(start);
    BrowseInput input;
    std::vector<float> gpu_vertices;
    std::vector<std::uint32_t> indices;
    const PresentCamera first = CameraFromPose(start);
    std::vector<std::uint8_t> image;
    result.drawn_pixels = Rasterize(first, meshes, &image, &gpu_vertices, &indices);
    if (gpu_vertices.empty() || indices.empty()) {
      result.message = "browse mesh produced no vertices";
      return result;
    }
    const size_t vertex_count = gpu_vertices.size();
    RunBrowseWindow(title, &input, gpu_vertices, indices, [&](std::vector<float>* vertices) {
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
        flight.elapsed_seconds += 0.016;
        pose = SampleFlight(flight);
        if (flight.elapsed_seconds >= flight.duration_seconds) {
          flight.active = false;
          orbit = OrbitFromPose(pose);
          result.flew = true;
        }
      }
      const PresentCamera frame = CameraFromPose(pose);
      std::vector<std::uint8_t> frame_image;
      std::vector<std::uint32_t> frame_indices;
      std::vector<float> projected;
      result.drawn_pixels =
          Rasterize(frame, meshes, &frame_image, &projected, &frame_indices);
      if (projected.size() != vertex_count) {
        result.message = "projected vertex count changed";
        return false;
      }
      *vertices = std::move(projected);
      result.eye[0] = frame.eye[0];
      result.eye[1] = frame.eye[1];
      result.eye[2] = frame.eye[2];
      ++result.frames;
      if (options.seconds > 0 && result.frames * 0.016 >= options.seconds) {
        if (flight.active && flight.elapsed_seconds + 0.02 >= flight.duration_seconds) {
          result.flew = true;
        }
        return false;
      }
      return true;
    });
    result.ok = result.frames > 0 && result.drawn_pixels > 200;
    if (result.message.empty()) {
      result.message = result.ok ? "browsed" : "browse produced no frames";
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
    if (result.drawn_pixels < 200 || gpu_vertices.empty() || indices.empty()) {
      result.message = "scene projected to too few pixels";
      return result;
    }
    ShowOnFlyCube(title, gpu_vertices, indices);
    result.ok = true;
    result.message = "presented";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

}  // namespace cesium_flycube
