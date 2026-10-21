// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/gpu/scene_present.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Instance/Instance.h"

#include <glm/vec4.hpp>

#include <windows.h>
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

NdcVertex Project(const PresentCamera& camera, Vec3 point) {
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
    return {0, 0, 1.0e30f, false};
  }
  const float fy = 1.0f / std::tan(camera.fov_y_radians * 0.5f);
  const float fx = fy / (static_cast<float>(kWidth) / static_cast<float>(kHeight));
  return {(fx * view_x) / view_z, (fy * view_y) / view_z, view_z, true};
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
                  const float color[4]) {
  if (!a.visible || !b.visible || !c.visible) {
    return;
  }
  const auto to_x = [](float ndc) {
    return (ndc * 0.5f + 0.5f) * static_cast<float>(kWidth);
  };
  const auto to_y = [](float ndc) {
    return (1.0f - (ndc * 0.5f + 0.5f)) * static_cast<float>(kHeight);
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
  const int min_x = std::max(0, static_cast<int>(std::floor(std::min({ax, bx, cx}))));
  const int max_x = std::min(kWidth - 1, static_cast<int>(std::ceil(std::max({ax, bx, cx}))));
  const int min_y = std::max(0, static_cast<int>(std::floor(std::min({ay, by, cy}))));
  const int max_y = std::min(kHeight - 1, static_cast<int>(std::ceil(std::max({ay, by, cy}))));
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
      const float inv = 1.0f / (w0 + w1 + w2);
      const float pixel_depth = (a.depth * w1 + b.depth * w2 + c.depth * w0) * inv;
      float& stored = (*depth)[static_cast<size_t>(y) * kWidth + x];
      if (pixel_depth >= stored) {
        continue;
      }
      stored = pixel_depth;
      if ((*image)[(static_cast<size_t>(y) * kWidth + x) * 4 + 3] == 0) {
        ++(*drawn);
      }
      PutPixel(image, x, y, color);
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

std::uint32_t Rasterize(const PresentCamera& camera,
                        const std::vector<PresentMesh>& meshes,
                        std::vector<std::uint8_t>* image,
                        std::vector<float>* ndc_xyz,
                        std::vector<std::uint32_t>* indices,
                        std::vector<std::uint32_t>* draws,
                        std::vector<glm::vec4>* colors) {
  image->assign(static_cast<size_t>(kWidth) * kHeight * 4, 0);
  std::vector<float> depth(static_cast<size_t>(kWidth) * kHeight, 1.0e30f);
  std::vector<float> edges;
  std::uint32_t drawn = 0;
  for (const PresentMesh& mesh : meshes) {
    const std::uint32_t base = static_cast<std::uint32_t>(ndc_xyz->size() / 3);
    std::vector<NdcVertex> projected;
    projected.reserve(mesh.mesh.xyz.size() / 3);
    for (size_t i = 0; i + 2 < mesh.mesh.xyz.size(); i += 3) {
      const NdcVertex vertex = Project(
          camera, {mesh.mesh.xyz[i], mesh.mesh.xyz[i + 1], mesh.mesh.xyz[i + 2]});
      projected.push_back(vertex);
      ndc_xyz->push_back(vertex.x);
      ndc_xyz->push_back(vertex.y);
      ndc_xyz->push_back(0.5f);
    }
    const std::uint32_t index_begin = static_cast<std::uint32_t>(indices->size());
    for (size_t i = 0; i + 2 < mesh.mesh.indices.size(); i += 3) {
      const std::uint32_t i0 = mesh.mesh.indices[i];
      const std::uint32_t i1 = mesh.mesh.indices[i + 1];
      const std::uint32_t i2 = mesh.mesh.indices[i + 2];
      if (i0 >= projected.size() || i1 >= projected.size() || i2 >= projected.size()) {
        continue;
      }
      FillTriangle(image, &depth, &drawn, projected[i0], projected[i1], projected[i2], mesh.color);
      const auto to_x = [](float ndc) {
        return (ndc * 0.5f + 0.5f) * static_cast<float>(kWidth);
      };
      const auto to_y = [](float ndc) {
        return (1.0f - (ndc * 0.5f + 0.5f)) * static_cast<float>(kHeight);
      };
      const NdcVertex& va = projected[i0];
      const NdcVertex& vb = projected[i1];
      const NdcVertex& vc = projected[i2];
      if (va.visible && vb.visible && vc.visible) {
        edges.insert(edges.end(), {to_x(va.x), to_y(va.y), to_x(vb.x), to_y(vb.y),
                                   to_x(vb.x), to_y(vb.y), to_x(vc.x), to_y(vc.y),
                                   to_x(vc.x), to_y(vc.y), to_x(va.x), to_y(va.y)});
      }
      indices->push_back(base + i0);
      indices->push_back(base + i1);
      indices->push_back(base + i2);
    }
    if (indices->size() > index_begin) {
      draws->push_back(index_begin);
      draws->push_back(static_cast<std::uint32_t>(indices->size() - index_begin));
      colors->push_back(glm::vec4(mesh.color[0], mesh.color[1], mesh.color[2], mesh.color[3]));
    }
  }
  for (size_t i = 0; i + 3 < edges.size(); i += 4) {
    DrawLine(image, edges[i], edges[i + 1], edges[i + 2], edges[i + 3]);
  }
  const float background[4] = {0.07f, 0.12f, 0.18f, 1.0f};
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      if ((*image)[(static_cast<size_t>(y) * kWidth + x) * 4 + 3] == 0) {
        PutPixel(image, x, y, background);
      }
    }
  }
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
                   const std::vector<float>& ndc_xyz,
                   const std::vector<std::uint32_t>& indices,
                   const std::vector<std::uint32_t>& draws,
                   const std::vector<glm::vec4>& colors) {
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
      BufferDesc{ndc_xyz.size() * sizeof(float), BindFlag::kVertexBuffer});
  vertex_buffer->UpdateUploadBuffer(0, ndc_xyz.data(), ndc_xyz.size() * sizeof(float));
  std::shared_ptr<Resource> index_buffer = device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{indices.size() * sizeof(std::uint32_t), BindFlag::kIndexBuffer});
  index_buffer->UpdateUploadBuffer(0, indices.data(), indices.size() * sizeof(std::uint32_t));

  std::vector<std::shared_ptr<Resource>> constants;
  std::vector<std::shared_ptr<View>> constant_views;
  std::vector<std::shared_ptr<BindingSet>> bindings;
  BindKey color_key{ShaderType::kPixel, ViewType::kConstantBuffer, 0, 0, 1};
  BindingSetLayoutDesc layout_desc;
  layout_desc.bind_keys = {color_key};
  std::shared_ptr<BindingSetLayout> layout = device->CreateBindingSetLayout(layout_desc);
  for (const glm::vec4& color : colors) {
    std::shared_ptr<Resource> constant = device->CreateBuffer(
        MemoryType::kUpload, BufferDesc{sizeof(glm::vec4), BindFlag::kConstantBuffer});
    constant->UpdateUploadBuffer(0, &color, sizeof(color));
    ViewDesc view_desc;
    view_desc.view_type = ViewType::kConstantBuffer;
    view_desc.dimension = ViewDimension::kBuffer;
    std::shared_ptr<View> view = device->CreateView(constant, view_desc);
    std::shared_ptr<BindingSet> set = device->CreateBindingSet(layout);
    WriteBindingsDesc write;
    write.bindings.push_back(BindingDesc{color_key, view});
    set->WriteBindings(write);
    constants.push_back(constant);
    constant_views.push_back(view);
    bindings.push_back(set);
  }

  const std::string shader_dir = CESIUM_FLYCUBE_SHADER_DIR;
  std::shared_ptr<Shader> vertex_shader = device->CompileShader(
      {shader_dir + "/SceneVs.hlsl", "main", ShaderType::kVertex, "6_0"});
  std::shared_ptr<Shader> pixel_shader = device->CompileShader(
      {shader_dir + "/ScenePs.hlsl", "main", ShaderType::kPixel, "6_0"});
  GraphicsPipelineDesc pipeline_desc;
  pipeline_desc.shaders = {vertex_shader, pixel_shader};
  pipeline_desc.layout = layout;
  pipeline_desc.input = {{0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 3}};
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
    for (size_t draw = 0; draw < draws.size(); draw += 2) {
      commands->BindBindingSet(bindings[draw / 2]);
      commands->DrawIndexed(draws[draw + 1], 1, draws[draw], 0, 0);
    }
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

}  // namespace

PresentResult PresentScene(const char* title,
                           const PresentCamera& camera,
                           const std::vector<PresentMesh>& meshes,
                           const std::string& png_path) {
  PresentResult result;
  result.png_path = png_path;
  try {
    std::vector<std::uint8_t> image;
    std::vector<float> ndc_xyz;
    std::vector<std::uint32_t> indices;
    std::vector<std::uint32_t> draws;
    std::vector<glm::vec4> colors;
    result.drawn_pixels = Rasterize(camera, meshes, &image, &ndc_xyz, &indices, &draws, &colors);
    std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());
    const std::wstring wide(png_path.begin(), png_path.end());
    if (!WritePng(wide, image)) {
      result.message = "png write failed";
      return result;
    }
    if (result.drawn_pixels < 200 || ndc_xyz.empty() || indices.empty()) {
      result.message = "scene projected to too few pixels";
      return result;
    }
    ShowOnFlyCube(title, ndc_xyz, indices, draws, colors);
    result.ok = true;
    result.message = "presented";
  } catch (const std::exception& error) {
    result.ok = false;
    result.message = error.what();
  }
  return result;
}

}  // namespace cesium_flycube
