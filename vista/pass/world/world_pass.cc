// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Cesium headers before Win32: Windows.h defines OPAQUE/TRANSPARENT macros that
// break CesiumGltf::Material::AlphaMode::OPAQUE.
#include "cesium_flycube/gpu/draped_terrain.h"
#include "cesium_flycube/gpu/scene_present.h"
#include "cesium_flycube/tiles/tileset_host.h"
#include "cesium_flycube/view/camera_flight.h"
#include "cesium_flycube/view/session.h"

#include <glm/geometric.hpp>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "vista/pass/world/world_pass.h"
#include "Instance/Instance.h"

#ifdef OPAQUE
#undef OPAQUE
#endif
#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace vista {

struct WorldPassImpl {
  // Local mesh vertex for sphere fallback (not a Cesium type).
  struct Vert {
    float x, y, z;
    float r, g, b;
  };

  HWND child = nullptr;
  int width = 0;
  int height = 0;
  double lon = 105.0;
  double lat = 35.0;
  double height_m = 1.2e7;

  bool draped = false;
  cesium_flycube::ProductMeshCache product_cache;
  cesium_flycube::OrbitController orbit;
  cesium_flycube::Flight flight;
  std::chrono::steady_clock::time_point last_frame =
      std::chrono::steady_clock::now();

  // Sphere fallback when CaptureDrapedEllipsoid fails.
  double yaw = 0.4;
  double pitch = 0.25;
  double distance = 1.84e7;
  std::vector<Vert> mesh;
  std::vector<uint32_t> indices;

  std::shared_ptr<Instance> instance;
  std::shared_ptr<Adapter> adapter;
  std::shared_ptr<Device> device;
  std::shared_ptr<CommandQueue> queue;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<Fence> fence;
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<Resource> vertex_buffer;
  std::shared_ptr<Resource> index_buffer;
  std::shared_ptr<BindingSetLayout> layout;
  std::vector<std::shared_ptr<CommandList>> command_lists;
  std::vector<uint64_t> frame_fences;
  uint64_t fence_value = 0;
  uint32_t frame_count = 3;
  bool ready = false;
};

namespace {

// Dense enough that the China mainland ring is readable on the sphere fallback.
constexpr int kStacks = 36;
constexpr int kSlices = 72;

using Session = WorldPassImpl;
using Vert = WorldPassImpl::Vert;

const wchar_t kClassName[] = L"VistaWorldPassDx12Child";

LRESULT CALLBACK ChildProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_ERASEBKGND)
    return 1;
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

bool EnsureClass() {
  static bool ready = false;
  if (ready)
    return true;
  WNDCLASSW wc = {};
  wc.lpfnWndProc = ChildProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.style = CS_OWNDC;
  if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    return false;
  ready = true;
  return true;
}

double EnvDouble(const char* name, double fallback) {
  char buf[64] = {};
  const DWORD n = GetEnvironmentVariableA(name, buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf))
    return fallback;
  return atof(buf);
}

bool EnvTruthy(const char* name) {
  char buf[32] = {};
  const DWORD n = GetEnvironmentVariableA(name, buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf))
    return false;
  return _stricmp(buf, "1") == 0 || _stricmp(buf, "true") == 0 ||
         _stricmp(buf, "yes") == 0 || _stricmp(buf, "imagery") == 0;
}

// Missing or empty → default_on; explicit 0/false/off → false.
bool EnvEnabled(const char* name, bool default_on) {
  char buf[32] = {};
  const DWORD n = GetEnvironmentVariableA(name, buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf))
    return default_on;
  if (_stricmp(buf, "0") == 0 || _stricmp(buf, "false") == 0 ||
      _stricmp(buf, "off") == 0 || _stricmp(buf, "no") == 0)
    return false;
  return true;
}

std::string EnvString(const char* name) {
  char buf[1024] = {};
  const DWORD n = GetEnvironmentVariableA(name, buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf))
    return std::string();
  return std::string(buf, n);
}

cesium_flycube::PresentCamera MakeCamera(const cesium_flycube::CameraPose& pose) {
  cesium_flycube::PresentCamera camera;
  camera.eye[0] = static_cast<float>(pose.eye[0]);
  camera.eye[1] = static_cast<float>(pose.eye[1]);
  camera.eye[2] = static_cast<float>(pose.eye[2]);
  camera.target[0] = static_cast<float>(pose.target[0]);
  camera.target[1] = static_cast<float>(pose.target[1]);
  camera.target[2] = static_cast<float>(pose.target[2]);
  camera.up[0] = static_cast<float>(pose.up[0]);
  camera.up[1] = static_cast<float>(pose.up[1]);
  camera.up[2] = static_cast<float>(pose.up[2]);
  camera.fov_y_radians = 0.65f;
  camera.draw_edges = false;
  camera.skip_shade = true;
  return camera;
}

void SyncGeodeticFromOrbit(Session* s) {
  s->height_m = s->orbit.distance;
  // Local ENU origin is the draped georeference; report configured lon/lat.
}

bool TryCaptureChina(cesium_flycube::Session* geo,
                     cesium_flycube::TilesetHost* tileset,
                     cesium_flycube::DrapedTerrainOptions opts,
                     cesium_flycube::DrapedTerrainResult* out) {
  *out = cesium_flycube::CaptureDrapedEllipsoid(geo, tileset, opts);
  return out->ok && !out->pictures.empty();
}

bool TryLoadDraped(Session* s) {
  cesium_flycube::Session geo;
  cesium_flycube::Camera overview;
  overview.viewport_size = glm::dvec2(960.0, 540.0);
  // Slightly wider China overview so the draped patch fills the viewport.
  overview.location = glm::dvec3(0.0, -3.5e5, 3.8e5);
  overview.direction = glm::normalize(glm::dvec3(0.0, 0.45, -1.0));
  overview.up = glm::dvec3(0.0, 0.0, 1.0);
  overview.horizontal_fov_degrees = 55.0;
  geo.cameras().additional_cameras() = {overview};

  cesium_flycube::DrapedTerrainOptions opts;
  opts.longitude_degrees = EnvDouble("MVIEW_FLYCUBE_LON", 105.0);
  opts.latitude_degrees = EnvDouble("MVIEW_FLYCUBE_LAT", 35.0);
  const bool offline = EnvTruthy("MVIEW_FLYCUBE_OFFLINE");
  // Product default: real DEM color + imagery. Offline / explicit 0 disables.
  opts.prefer_imagery = offline ? EnvTruthy("MVIEW_FLYCUBE_IMAGERY")
                                : EnvEnabled("MVIEW_FLYCUBE_IMAGERY", true);
  opts.imagery_url_template = EnvString("MVIEW_FLYCUBE_IMAGERY_URL");
  opts.prefer_dem = offline ? false : EnvEnabled("MVIEW_FLYCUBE_DEM", true);
  opts.dem_url_template = EnvString("MVIEW_FLYCUBE_DEM_URL");
  opts.max_ticks =
      (opts.prefer_imagery || opts.prefer_dem) ? 260 : 180;
  opts.require_drape = true;

  s->lon = opts.longitude_degrees;
  s->lat = opts.latitude_degrees;

  cesium_flycube::TilesetHost tileset;
  cesium_flycube::DrapedTerrainResult terrain;
  if (!TryCaptureChina(&geo, &tileset, opts, &terrain)) {
    // Ellipsoid mesh with normal shading is still a usable China DEM patch
    // when raster drape (debug colorize / imagery) times out.
    opts.require_drape = false;
    opts.max_ticks = 200;
    cesium_flycube::TilesetHost tileset2;
    if (!TryCaptureChina(&geo, &tileset2, opts, &terrain))
      return false;
  }

  cesium_flycube::PresentCamera style = {};
  style.skip_shade = true;
  if (!cesium_flycube::BuildProductMeshCache(style, terrain.pictures,
                                             &s->product_cache)) {
    return false;
  }

  s->orbit.target[0] = 0;
  s->orbit.target[1] = 0;
  s->orbit.target[2] = 0;
  // Start at a China-region overview (not a full-Earth disk).
  s->orbit.distance = 4.5e5;
  s->orbit.yaw_radians = 0.15;
  s->orbit.pitch_radians = 0.55;
  s->orbit.max_distance = 4.0e7;
  s->orbit.min_distance = 80.0;

  cesium_flycube::OrbitController near_orbit = s->orbit;
  near_orbit.distance = 4.0e4;
  near_orbit.yaw_radians = 0.45;
  near_orbit.pitch_radians = 0.52;
  cesium_flycube::BeginFlight(&s->flight, cesium_flycube::PoseFromOrbit(s->orbit),
                              cesium_flycube::PoseFromOrbit(near_orbit), 5.0);
  s->height_m = s->orbit.distance;
  s->draped = true;
  s->indices = s->product_cache.indices;
  return true;
}

// Simplified mainland outline for sphere fallback (same ring as MapLibre).
const float kChinaLon[] = {
    73.5f, 76.0f, 80.0f, 85.0f, 87.5f, 90.0f, 93.0f, 97.0f, 100.0f, 104.0f,
    108.0f, 111.5f, 115.0f, 119.0f, 122.5f, 126.0f, 130.0f, 134.0f, 133.0f,
    131.0f, 128.5f, 126.0f, 124.0f, 122.5f, 121.8f, 121.5f, 121.8f, 122.0f,
    121.5f, 121.0f, 120.2f, 119.2f, 118.0f, 116.5f, 114.5f, 112.5f, 110.5f,
    109.0f, 107.5f, 106.0f, 104.5f, 102.5f, 100.5f, 98.8f, 97.8f, 97.2f,
    94.5f, 91.5f, 88.0f, 85.0f, 82.0f, 79.5f, 77.5f, 75.5f, 73.5f};
const float kChinaLat[] = {
    39.0f, 40.5f, 42.0f, 45.0f, 47.0f, 48.5f, 49.5f, 50.5f, 51.5f, 53.0f,
    53.2f, 51.0f, 48.5f, 47.2f, 47.5f, 48.8f, 48.5f, 48.0f, 46.0f, 45.0f,
    43.0f, 41.5f, 40.2f, 39.5f, 38.0f, 36.5f, 34.5f, 32.0f, 30.5f, 29.0f,
    27.5f, 26.0f, 24.5f, 23.5f, 22.5f, 21.8f, 21.2f, 21.3f, 21.8f, 22.8f,
    22.6f, 22.0f, 21.5f, 23.5f, 26.5f, 28.2f, 28.8f, 28.2f, 28.0f, 28.5f,
    29.5f, 31.0f, 33.0f, 35.5f, 39.0f};
const int kChinaN = static_cast<int>(sizeof(kChinaLon) / sizeof(kChinaLon[0]));

bool PointInChina(float lon_deg, float lat_deg) {
  // Ray cast against mainland ring.
  bool inside = false;
  for (int i = 0, j = kChinaN - 1; i < kChinaN; j = i++) {
    const float yi = kChinaLat[i];
    const float yj = kChinaLat[j];
    const float xi = kChinaLon[i];
    const float xj = kChinaLon[j];
    const bool hit = ((yi > lat_deg) != (yj > lat_deg)) &&
                     (lon_deg < (xj - xi) * (lat_deg - yi) / (yj - yi + 1e-6f) + xi);
    if (hit)
      inside = !inside;
  }
  // Taiwan / Hainan boxes (readable on a low-res sphere).
  if (lon_deg > 120.3f && lon_deg < 122.3f && lat_deg > 21.6f && lat_deg < 25.5f)
    return true;
  if (lon_deg > 108.4f && lon_deg < 111.3f && lat_deg > 18.0f && lat_deg < 20.3f)
    return true;
  return inside;
}

// Fake DEM height (0..1) over China for sphere-fallback shading.
float ChinaDemShade(float lon_deg, float lat_deg) {
  // Tibetan plateau / west high, east low — readable green→brown ramp.
  const float west = (std::max)(0.0f, (std::min)(1.0f, (100.0f - lon_deg) / 25.0f));
  const float north = (std::max)(0.0f, (std::min)(1.0f, (lat_deg - 25.0f) / 25.0f));
  return 0.25f + 0.55f * west + 0.20f * north;
}

void BuildSphere(std::vector<Vert>* verts, std::vector<uint32_t>* indices) {
  verts->clear();
  indices->clear();
  verts->reserve(static_cast<size_t>(kStacks + 1) * (kSlices + 1));
  constexpr double kR = 6371000.0;
  for (int y = 0; y <= kStacks; ++y) {
    const float v = static_cast<float>(y) / static_cast<float>(kStacks);
    const float lat = (0.5f - v) * 3.14159265f;
    const float cos_lat = std::cos(lat);
    const float sin_lat = std::sin(lat);
    const float lat_deg = lat * 180.0f / 3.14159265f;
    for (int x = 0; x <= kSlices; ++x) {
      const float u = static_cast<float>(x) / static_cast<float>(kSlices);
      const float lon = u * 2.0f * 3.14159265f - 3.14159265f;
      const float lon_deg = lon * 180.0f / 3.14159265f;
      Vert vert;
      vert.x = static_cast<float>(kR * cos_lat * std::cos(lon));
      vert.y = static_cast<float>(kR * sin_lat);
      vert.z = static_cast<float>(kR * cos_lat * std::sin(lon));
      if (PointInChina(lon_deg, lat_deg)) {
        const float h = ChinaDemShade(lon_deg, lat_deg);
        vert.r = 0.20f + 0.35f * h;
        vert.g = 0.55f - 0.25f * h;
        vert.b = 0.22f + 0.05f * h;
      } else {
        vert.r = 0.14f;
        vert.g = 0.32f;
        vert.b = 0.55f;
      }
      verts->push_back(vert);
    }
  }
  for (int y = 0; y < kStacks; ++y) {
    for (int x = 0; x < kSlices; ++x) {
      const uint32_t i0 = static_cast<uint32_t>(y * (kSlices + 1) + x);
      const uint32_t i1 = i0 + static_cast<uint32_t>(kSlices + 1);
      indices->push_back(i0);
      indices->push_back(i1);
      indices->push_back(i1 + 1);
      indices->push_back(i0);
      indices->push_back(i1 + 1);
      indices->push_back(i0 + 1);
    }
  }
}

void SyncSphereDistance(Session* s) {
  s->yaw = s->lon * 3.14159265 / 180.0;
  s->pitch = s->lat * 3.14159265 / 180.0 * 0.35;
  if (s->pitch > 1.2)
    s->pitch = 1.2;
  if (s->pitch < -1.2)
    s->pitch = -1.2;
  constexpr double kEarthR = 6371000.0;
  s->distance = std::max(kEarthR + 800.0, kEarthR + s->height_m);
}

void ProjectSphere(const Session& s, std::vector<float>* gpu_vertices) {
  const float cy = static_cast<float>(std::cos(s.yaw));
  const float sy = static_cast<float>(std::sin(s.yaw));
  const float cp = static_cast<float>(std::cos(s.pitch));
  const float sp = static_cast<float>(std::sin(s.pitch));
  float eye[3] = {static_cast<float>(s.distance) * cp * cy,
                  static_cast<float>(s.distance) * sp,
                  static_cast<float>(s.distance) * cp * sy};
  float target[3] = {0, 0, 0};
  float up[3] = {0, 1, 0};
  float forward[3] = {target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]};
  float fl = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] +
                       forward[2] * forward[2]);
  if (fl < 1.0f)
    fl = 1.0f;
  forward[0] /= fl;
  forward[1] /= fl;
  forward[2] /= fl;
  float right[3] = {forward[1] * up[2] - forward[2] * up[1],
                    forward[2] * up[0] - forward[0] * up[2],
                    forward[0] * up[1] - forward[1] * up[0]};
  float rl = std::sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
  if (rl < 1e-6f)
    rl = 1.0f;
  right[0] /= rl;
  right[1] /= rl;
  right[2] /= rl;
  float cup[3] = {right[1] * forward[2] - right[2] * forward[1],
                  right[2] * forward[0] - right[0] * forward[2],
                  right[0] * forward[1] - right[1] * forward[0]};
  const float aspect =
      s.height > 0 ? static_cast<float>(s.width) / static_cast<float>(s.height)
                   : 1.0f;
  const float f = 1.0f / std::tan(0.45f);
  gpu_vertices->clear();
  gpu_vertices->reserve(s.mesh.size() * 6);
  for (const Vert& v : s.mesh) {
    const float dx = v.x - eye[0];
    const float dy = v.y - eye[1];
    const float dz = v.z - eye[2];
    const float cam_x = dx * right[0] + dy * right[1] + dz * right[2];
    const float cam_y = dx * cup[0] + dy * cup[1] + dz * cup[2];
    const float cam_z = dx * forward[0] + dy * forward[1] + dz * forward[2];
    const float w = (std::max)(cam_z, 1.0f);
    gpu_vertices->push_back((cam_x * f / aspect) / w);
    gpu_vertices->push_back((cam_y * f) / w);
    gpu_vertices->push_back(cam_z / (static_cast<float>(s.distance) * 2.0f));
    gpu_vertices->push_back(v.r);
    gpu_vertices->push_back(v.g);
    gpu_vertices->push_back(v.b);
  }
}

void ProjectFrame(Session* s, std::vector<float>* gpu_vertices) {
  if (s->draped) {
    const auto now = std::chrono::steady_clock::now();
    const double dt =
        std::chrono::duration<double>(now - s->last_frame).count();
    s->last_frame = now;
    cesium_flycube::CameraPose pose = cesium_flycube::PoseFromOrbit(s->orbit);
    if (s->flight.active) {
      s->flight.elapsed_seconds += dt;
      pose = cesium_flycube::SampleFlight(s->flight);
      if (s->flight.elapsed_seconds >= s->flight.duration_seconds) {
        s->flight.active = false;
        s->orbit = cesium_flycube::OrbitFromPose(pose);
      }
    }
    SyncGeodeticFromOrbit(s);
    cesium_flycube::ProjectProductMeshCache(MakeCamera(pose), s->product_cache,
                                            gpu_vertices);
    return;
  }
  ProjectSphere(*s, gpu_vertices);
}

bool CreateDx12Inner(Session* s, char* error, size_t error_bytes) {
  s->instance = CreateInstance(ApiType::kDX12);
  if (!s->instance) {
    std::snprintf(error, error_bytes, "CreateInstance null");
    return false;
  }
  const auto adapters = s->instance->EnumerateAdapters();
  if (adapters.empty()) {
    std::snprintf(error, error_bytes, "no adapters");
    return false;
  }
  s->adapter = adapters[0];
  s->device = s->adapter->CreateDevice();
  if (!s->device) {
    std::snprintf(error, error_bytes, "CreateDevice null");
    return false;
  }
  return true;
}

bool TryCreateDx12(Session* s) {
  char message[128] = {};
  bool ok = false;
  __try {
    ok = CreateDx12Inner(s, message, sizeof(message));
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    ok = false;
  }
  return ok;
}

void RebuildCommandLists(Session* s);

bool InitGpu(Session* s) {
  if (!TryCreateDx12(s))
    return false;
  s->queue = s->device->GetCommandQueue(CommandListType::kGraphics);
  Win32Surface win32{GetModuleHandleW(nullptr), s->child};
  s->swapchain =
      s->device->CreateSwapchain(win32, s->width, s->height, s->frame_count, false);
  if (!s->swapchain)
    return false;
  s->fence = s->device->CreateFence(0);
  s->frame_fences.assign(s->frame_count, 0);

  BindingSetLayoutDesc layout_desc;
  s->layout = s->device->CreateBindingSetLayout(layout_desc);
  const std::string shader_dir = CESIUM_FLYCUBE_SHADER_DIR;
  std::shared_ptr<Shader> vs = s->device->CompileShader(
      {shader_dir + "/SceneVs.hlsl", "main", ShaderType::kVertex, "6_0"});
  std::shared_ptr<Shader> ps = s->device->CompileShader(
      {shader_dir + "/ScenePs.hlsl", "main", ShaderType::kPixel, "6_0"});
  if (!vs || !ps)
    return false;
  GraphicsPipelineDesc pipeline_desc;
  pipeline_desc.shaders = {vs, ps};
  pipeline_desc.layout = s->layout;
  pipeline_desc.input = {
      {0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6, 0},
      {0, "COLOR", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(float) * 6,
       sizeof(float) * 3},
  };
  pipeline_desc.color_formats = {s->swapchain->GetFormat()};
  pipeline_desc.depth_stencil_desc.depth_test_enable = false;
  pipeline_desc.depth_stencil_desc.depth_write_enable = false;
  s->pipeline = s->device->CreateGraphicsPipeline(pipeline_desc);
  if (!s->pipeline)
    return false;

  if (!s->draped) {
    BuildSphere(&s->mesh, &s->indices);
    SyncSphereDistance(s);
  }

  std::vector<float> gpu;
  ProjectFrame(s, &gpu);
  s->vertex_buffer = s->device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{gpu.size() * sizeof(float), BindFlag::kVertexBuffer});
  s->vertex_buffer->UpdateUploadBuffer(0, gpu.data(), gpu.size() * sizeof(float));
  s->index_buffer = s->device->CreateBuffer(
      MemoryType::kUpload,
      BufferDesc{s->indices.size() * sizeof(uint32_t), BindFlag::kIndexBuffer});
  s->index_buffer->UpdateUploadBuffer(0, s->indices.data(),
                                      s->indices.size() * sizeof(uint32_t));
  RebuildCommandLists(s);
  s->ready = true;
  return true;
}

void RebuildCommandLists(Session* s) {
  if (!s->swapchain)
    return;
  s->command_lists.clear();
  s->command_lists.resize(s->frame_count);
  for (uint32_t frame = 0; frame < s->frame_count; ++frame) {
    std::shared_ptr<Resource> back = s->swapchain->GetBackBuffer(frame);
    ViewDesc target_desc{ViewType::kRenderTarget, ViewDimension::kTexture2D};
    std::shared_ptr<View> target = s->device->CreateView(back, target_desc);
    std::shared_ptr<CommandList> commands =
        s->device->CreateCommandList(CommandListType::kGraphics);
    commands->ResourceBarrier(
        {{back, ResourceState::kPresent, ResourceState::kRenderTarget}});
    RenderPassDesc pass;
    pass.render_area = {0, 0, static_cast<uint32_t>(s->width),
                        static_cast<uint32_t>(s->height)};
    pass.colors.push_back(RenderPassColorDesc{
        target, RenderPassLoadOp::kClear, RenderPassStoreOp::kStore,
        {0.07f, 0.12f, 0.18f, 1.0f}});
    commands->BeginRenderPass(pass);
    commands->BindPipeline(s->pipeline);
    commands->SetViewport(0, 0, static_cast<float>(s->width),
                          static_cast<float>(s->height), 0.0f, 1.0f);
    commands->SetScissorRect(0, 0, s->width, s->height);
    commands->IASetIndexBuffer(s->index_buffer, 0,
                               gli::format::FORMAT_R32_UINT_PACK32);
    commands->IASetVertexBuffer(0, s->vertex_buffer, 0);
    commands->DrawIndexed(static_cast<uint32_t>(s->indices.size()), 1, 0, 0, 0);
    commands->EndRenderPass();
    commands->ResourceBarrier(
        {{back, ResourceState::kRenderTarget, ResourceState::kPresent}});
    commands->Close();
    s->command_lists[frame] = commands;
  }
}

}  // namespace

WorldPass::WorldPass() : impl_(nullptr) {}

WorldPass::~WorldPass() {
  Destroy();
}

bool WorldPass::Create(HWND parent, int width, int height) {
  Destroy();
  if (!parent || !IsWindow(parent) || !EnsureClass())
    return false;
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  auto* s = new Session();
  s->width = width;
  s->height = height;
  s->child = CreateWindowExW(0, kClassName, L"",
                             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0,
                             width, height, parent, nullptr,
                             GetModuleHandleW(nullptr), nullptr);
  if (!s->child) {
    delete s;
    return false;
  }
  if (!TryLoadDraped(s)) {
    s->draped = false;
    SyncSphereDistance(s);
  }
  if (!InitGpu(s)) {
    DestroyWindow(s->child);
    delete s;
    return false;
  }
  impl_ = s;
  return true;
}

void WorldPass::Destroy() {
  auto* s = impl_;
  if (!s)
    return;
  if (s->queue && s->fence) {
    s->queue->Signal(s->fence, ++s->fence_value);
    s->fence->Wait(s->fence_value);
  }
  s->command_lists.clear();
  s->pipeline.reset();
  s->vertex_buffer.reset();
  s->index_buffer.reset();
  s->swapchain.reset();
  s->fence.reset();
  s->queue.reset();
  s->device.reset();
  s->adapter.reset();
  s->instance.reset();
  if (s->child)
    DestroyWindow(s->child);
  delete s;
  impl_ = nullptr;
}

HWND WorldPass::hwnd() const {
  return impl_ ? impl_->child : nullptr;
}

void WorldPass::Resize(int width, int height) {
  auto* s = impl_;
  if (!s || !s->child || !IsWindow(s->child))
    return;
  RECT client = {};
  if (GetClientRect(s->child, &client)) {
    const int cw = client.right - client.left;
    const int ch = client.bottom - client.top;
    if (cw >= 2 && ch >= 2) {
      width = cw;
      height = ch;
    }
  }
  if (width < 2)
    width = 2;
  if (height < 2)
    height = 2;
  if (s->width == width && s->height == height && s->swapchain)
    return;
  s->width = width;
  s->height = height;
  if (!s->ready || !s->device)
    return;
  if (s->queue && s->fence) {
    s->queue->Signal(s->fence, ++s->fence_value);
    s->fence->Wait(s->fence_value);
  }
  s->command_lists.clear();
  s->swapchain.reset();
  Win32Surface win32{GetModuleHandleW(nullptr), s->child};
  try {
    s->swapchain = s->device->CreateSwapchain(
        win32, s->width, s->height, s->frame_count, false);
  } catch (...) {
    s->swapchain.reset();
  }
  if (!s->swapchain)
    return;
  s->frame_fences.assign(s->frame_count, 0);
  RebuildCommandLists(s);
}

bool WorldPass::Present() {
  auto* s = impl_;
  if (!s || !s->ready || !s->swapchain)
    return false;
  std::vector<float> gpu;
  ProjectFrame(s, &gpu);
  if (gpu.empty() || s->indices.empty())
    return false;
  s->vertex_buffer->UpdateUploadBuffer(0, gpu.data(), gpu.size() * sizeof(float));
  for (uint64_t waited : s->frame_fences)
    s->fence->Wait(waited);
  const uint32_t image = s->swapchain->NextImage(s->fence, ++s->fence_value);
  s->queue->Wait(s->fence, s->fence_value);
  s->fence->Wait(s->frame_fences[image]);
  s->queue->ExecuteCommandLists({s->command_lists[image]});
  s->queue->Signal(s->fence, s->frame_fences[image] = ++s->fence_value);
  s->swapchain->Present(s->fence, s->frame_fences[image]);
  return true;
}

void WorldPass::Orbit(double dx_px, double dy_px) {
  auto* s = impl_;
  if (!s)
    return;
  if (s->draped) {
    if (s->flight.active) {
      s->orbit = cesium_flycube::OrbitFromPose(
          cesium_flycube::SampleFlight(s->flight));
      s->flight.active = false;
    }
    cesium_flycube::DragOrbit(&s->orbit, dx_px, dy_px);
    SyncGeodeticFromOrbit(s);
    return;
  }
  s->lon += dx_px * 0.15;
  s->lat += dy_px * 0.10;
  if (s->lat > 85.0)
    s->lat = 85.0;
  if (s->lat < -85.0)
    s->lat = -85.0;
  SyncSphereDistance(s);
}

void WorldPass::SetCamera(double lon, double lat, double height_m) {
  auto* s = impl_;
  if (!s)
    return;
  s->lon = lon;
  s->lat = lat;
  s->height_m = height_m;
  if (s->draped) {
    s->flight.active = false;
    // Draped mesh is a local ENU China patch; full-globe heights make it a speck.
    double d = height_m;
    if (d < s->orbit.min_distance)
      d = s->orbit.min_distance;
    if (d > 2.5e6)
      d = 2.5e6;
    if (d > s->orbit.max_distance)
      d = s->orbit.max_distance;
    s->orbit.distance = d;
    SyncGeodeticFromOrbit(s);
    return;
  }
  SyncSphereDistance(s);
}

void WorldPass::GetCamera(double* lon, double* lat, double* height_m) const {
  auto* s = impl_;
  if (!s)
    return;
  if (lon)
    *lon = s->lon;
  if (lat)
    *lat = s->lat;
  if (height_m)
    *height_m = s->draped ? s->orbit.distance : s->height_m;
}

}  // namespace vista
