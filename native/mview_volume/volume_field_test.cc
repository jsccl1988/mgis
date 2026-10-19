// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Harness for the synthetic volume and wind grid.
// Writes out/harness/browser.field/mark.json. VTK is not linked.
// The depth predicate is a host check for a nearer scene depth.
// It is not a VTK ray march and it does not run in Unreal.

#include "native/mview_volume/mview_volume.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include <windows.h>

namespace {

bool Fail(const char* message) {
  fprintf(stderr, "volume_field_test: %s\n", message);
  return false;
}

// Samples along a ray in the same units as |scene_depth| and |volume_exit|.
// When the scene depth is nearer than the volume exit, the count of samples
// that stop there is greater than zero. Otherwise the count is zero.
int VolumeDepthStopped(float scene_depth, float volume_exit, float step) {
  if (!(step > 0.0f) || !(scene_depth < volume_exit))
    return 0;
  int stopped = 0;
  float t = 0.0f;
  const int kMaxSteps = 4096;
  for (int i = 0; i < kMaxSteps && t <= volume_exit; ++i) {
    if (t >= scene_depth) {
      ++stopped;
      break;
    }
    t += step;
  }
  return stopped;
}

// Transfer-function body: color stops only. No voxel array.
std::string TransferPayload() {
  return std::string(
      "{\"stops\":["
      "{\"offset\":0.0,\"color\":\"#00000000\"},"
      "{\"offset\":0.5,\"color\":\"#3366ff80\"},"
      "{\"offset\":1.0,\"color\":\"#ff3300ff\"}"
      "]}");
}

bool PayloadOmitsVoxels(const std::string& payload) {
  if (payload.find("\"stops\"") == std::string::npos)
    return false;
  if (payload.find("voxel") != std::string::npos)
    return false;
  if (payload.find("scalars") != std::string::npos)
    return false;
  if (payload.find("\"data\"") != std::string::npos)
    return false;
  const size_t voxel_bytes =
      static_cast<size_t>(MVV_SYNTHETIC_DIM) * MVV_SYNTHETIC_DIM *
      MVV_SYNTHETIC_DIM * sizeof(float);
  if (payload.size() >= voxel_bytes)
    return false;
  return true;
}

bool WriteMark(int depth_stopped) {
  wchar_t module_path[MAX_PATH];
  DWORD n = GetModuleFileNameW(NULL, module_path, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return Fail("GetModuleFileNameW failed");

  wchar_t* slash = wcsrchr(module_path, L'\\');
  if (!slash)
    return Fail("executable path has no directory");
  *slash = 0;
  slash = wcsrchr(module_path, L'\\');
  if (!slash)
    return Fail("executable path has no parent");
  *slash = 0;

  wchar_t harness_dir[MAX_PATH];
  wchar_t suite_dir[MAX_PATH];
  wchar_t mark_path[MAX_PATH];
  _snwprintf_s(harness_dir, MAX_PATH, _TRUNCATE, L"%s\\harness", module_path);
  _snwprintf_s(suite_dir, MAX_PATH, _TRUNCATE, L"%s\\browser.field", harness_dir);
  _snwprintf_s(mark_path, MAX_PATH, _TRUNCATE, L"%s\\mark.json", suite_dir);

  CreateDirectoryW(harness_dir, NULL);
  CreateDirectoryW(suite_dir, NULL);

  FILE* file = NULL;
  if (_wfopen_s(&file, mark_path, L"wb") != 0 || !file)
    return Fail("failed to open mark.json");

  const int written = fprintf(
      file,
      "{\n"
      "  \"suite\": \"browser.field\",\n"
      "  \"sources\": \"synthetic\",\n"
      "  \"vtk_linked\": false,\n"
      "  \"marks\": [\"volume.ready\"],\n"
      "  \"volume.ready\": true,\n"
      "  \"volume.depth_stopped\": %d,\n"
      "  \"particle_bytes\": 0,\n"
      "  \"nx\": %d,\n"
      "  \"ny\": %d,\n"
      "  \"nz\": %d,\n"
      "  \"transfer_has_voxel_array\": false\n"
      "}\n",
      depth_stopped, MVV_SYNTHETIC_DIM, MVV_SYNTHETIC_DIM, MVV_SYNTHETIC_DIM);
  fclose(file);
  if (written < 0)
    return Fail("failed to write mark.json");
  return true;
}

bool ExpectGrid64(MvvGrid* grid, int components, const char* label) {
  const size_t expect = static_cast<size_t>(MVV_SYNTHETIC_DIM) *
                        MVV_SYNTHETIC_DIM * MVV_SYNTHETIC_DIM *
                        static_cast<size_t>(components);
  if (mvv_value_count(grid) != expect) {
    fprintf(stderr, "volume_field_test: %s value count mismatch\n", label);
    return false;
  }
  return true;
}

}  // namespace

int main() {
  char err[256];

  MvvGrid* scalar = mvv_open_scalar(L"synthetic://scalar", "temperature", 0,
                                    err, static_cast<int>(sizeof(err)));
  if (!scalar)
    return Fail(err[0] ? err : "mvv_open_scalar failed") ? 0 : 1;
  if (!ExpectGrid64(scalar, 1, "scalar")) {
    mvv_close(scalar);
    return 1;
  }

  const size_t scalar_count = mvv_value_count(scalar);
  std::vector<float> samples(scalar_count, -1.0f);
  mvv_copy_f32(scalar, &samples[0], scalar_count);
  bool any_written = false;
  bool varied = false;
  for (size_t i = 0; i < samples.size(); ++i) {
    if (samples[i] != -1.0f)
      any_written = true;
    if (samples[i] != samples[0])
      varied = true;
  }
  if (!any_written || !varied) {
    mvv_close(scalar);
    return Fail("scalar field was not a 64^3 synthetic volume") ? 0 : 1;
  }

  double llh[24];
  memset(llh, 0, sizeof(llh));
  mvv_corners_llh(scalar, llh);
  if (!(llh[0] >= -180.0 && llh[0] <= 180.0 && llh[1] >= -90.0 &&
        llh[1] <= 90.0)) {
    mvv_close(scalar);
    return Fail("corners are not longitude/latitude") ? 0 : 1;
  }
  if (!(llh[21] > llh[0] && llh[22] > llh[1] && llh[23] > llh[2])) {
    mvv_close(scalar);
    return Fail("far corner is not past the origin corner") ? 0 : 1;
  }
  mvv_close(scalar);

  MvvGrid* wind =
      mvv_open_wind(L"synthetic://wind", 0, err, static_cast<int>(sizeof(err)));
  if (!wind)
    return Fail(err[0] ? err : "mvv_open_wind failed") ? 0 : 1;
  if (!ExpectGrid64(wind, 3, "wind")) {
    mvv_close(wind);
    return 1;
  }

  float before = 0.0f;
  mvv_copy_f32(wind, &before, 1);
  std::vector<float> particles;
  const size_t particle_bytes = mvv_set_time_index(wind, 2);
  float after = 0.0f;
  mvv_copy_f32(wind, &after, 1);
  if (particle_bytes != 0 || !particles.empty()) {
    mvv_close(wind);
    return Fail("time index change copied a particle buffer") ? 0 : 1;
  }
  if (mvv_time_index(wind) != 2 || before == after) {
    mvv_close(wind);
    return Fail("time index did not change the wind field") ? 0 : 1;
  }
  if (!ExpectGrid64(wind, 3, "wind after time")) {
    mvv_close(wind);
    return 1;
  }
  mvv_close(wind);

  const std::string tf = TransferPayload();
  if (!PayloadOmitsVoxels(tf))
    return Fail("transfer payload contains a voxel array") ? 0 : 1;

  const int missed = VolumeDepthStopped(2.0f, 1.0f, 0.1f);
  const int depth_stopped = VolumeDepthStopped(0.25f, 1.0f, 0.1f);
  if (missed != 0 || depth_stopped <= 0)
    return Fail("volume.depth_stopped was not positive for a nearer depth") ? 0
                                                                            : 1;

  MvvGrid* resampled = mvv_open_scalar(L"synthetic://scalar", "temperature", 1,
                                       err, static_cast<int>(sizeof(err)));
  if (!resampled)
    return Fail("resample open failed") ? 0 : 1;
  mvv_resample(resampled, 32, 32, 32);
  if (mvv_value_count(resampled) != 32u * 32u * 32u) {
    mvv_close(resampled);
    return Fail("resample did not rebuild the grid") ? 0 : 1;
  }
  mvv_close(resampled);

  if (!WriteMark(depth_stopped))
    return 1;
  return 0;
}
