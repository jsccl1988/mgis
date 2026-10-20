// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Synthetic regular grids for the volume and wind C API.
// third_party/vtk is an example only: VTK headers and libraries are not
// in this tree, and this target does not link them.

#include "third_party/vtk/mview_volume/mview_volume.h"

#include <string.h>

#include <vector>

namespace {

const int kBudget = 256;
const int kWindComponents = 3;

const double kLon0 = 116.39;
const double kLat0 = 39.90;
const double kHeight0 = 50.0;
const double kLonExtent = 0.02;
const double kLatExtent = 0.02;
const double kHeightExtent = 500.0;

void SetErr(char* err, int err_cap, const char* message) {
  if (!err || err_cap <= 0 || !message)
    return;
  size_t cap = static_cast<size_t>(err_cap);
  size_t n = strlen(message);
  if (n >= cap)
    n = cap - 1;
  memcpy(err, message, n);
  err[n] = '\0';
}

void ClearErr(char* err, int err_cap) {
  if (err && err_cap > 0)
    err[0] = '\0';
}

int ClampDim(int n) {
  if (n < 1)
    return 1;
  if (n > kBudget)
    return kBudget;
  return n;
}

float ScalarAt(int x, int y, int z, int time_index) {
  int mixed = x * 3 + y * 5 + z * 7 + time_index * 11;
  int wrapped = mixed % 251;
  if (wrapped < 0)
    wrapped = -wrapped;
  return static_cast<float>(wrapped) / 250.0f;
}

}  // namespace

// Owns one regular float grid. Wind stores U then V then W.
// There is no particle storage; time changes only rewrite these floats.
struct MvvGrid {
  int nx;
  int ny;
  int nz;
  int components;
  int time_index;
  std::vector<float> values;

  MvvGrid() : nx(0), ny(0), nz(0), components(1), time_index(0) {}
};

namespace {

void Fill(MvvGrid* grid) {
  const int nx = grid->nx;
  const int ny = grid->ny;
  const int nz = grid->nz;
  const int comps = grid->components;
  const size_t voxels = static_cast<size_t>(nx) * static_cast<size_t>(ny) *
                        static_cast<size_t>(nz);
  grid->values.resize(voxels * static_cast<size_t>(comps));
  for (int z = 0; z < nz; ++z) {
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        const size_t voxel = (static_cast<size_t>(z) * static_cast<size_t>(ny) +
                              static_cast<size_t>(y)) *
                                 static_cast<size_t>(nx) +
                             static_cast<size_t>(x);
        const float s = ScalarAt(x, y, z, grid->time_index);
        if (comps == 1) {
          grid->values[voxel] = s;
        } else {
          grid->values[voxel] = s;
          grid->values[voxels + voxel] = ScalarAt(y, z, x, grid->time_index + 1);
          grid->values[2 * voxels + voxel] =
              ScalarAt(z, x, y, grid->time_index + 2) * 0.25f;
        }
      }
    }
  }
}

MvvGrid* MakeGrid(int components, int time_index) {
  MvvGrid* grid = new MvvGrid();
  grid->nx = MVV_SYNTHETIC_DIM;
  grid->ny = MVV_SYNTHETIC_DIM;
  grid->nz = MVV_SYNTHETIC_DIM;
  grid->components = components;
  grid->time_index = time_index;
  Fill(grid);
  return grid;
}

}  // namespace

MvvGrid* mvv_open_scalar(const wchar_t* path,
                         const char* variable,
                         int time_index,
                         char* err,
                         int err_cap) {
  if (!path || path[0] == L'\0') {
    SetErr(err, err_cap, "path is empty");
    return NULL;
  }
  if (!variable || variable[0] == '\0') {
    SetErr(err, err_cap, "variable is empty");
    return NULL;
  }
  if (time_index < 0) {
    SetErr(err, err_cap, "time_index is negative");
    return NULL;
  }
  ClearErr(err, err_cap);
  return MakeGrid(1, time_index);
}

MvvGrid* mvv_open_wind(const wchar_t* path,
                       int time_index,
                       char* err,
                       int err_cap) {
  if (!path || path[0] == L'\0') {
    SetErr(err, err_cap, "path is empty");
    return NULL;
  }
  if (time_index < 0) {
    SetErr(err, err_cap, "time_index is negative");
    return NULL;
  }
  ClearErr(err, err_cap);
  return MakeGrid(kWindComponents, time_index);
}

void mvv_resample(MvvGrid* grid, int nx, int ny, int nz) {
  if (!grid)
    return;
  if (nx < 1 || ny < 1 || nz < 1)
    return;
  grid->nx = ClampDim(nx);
  grid->ny = ClampDim(ny);
  grid->nz = ClampDim(nz);
  Fill(grid);
}

void mvv_copy_f32(MvvGrid* grid, float* dst, size_t count) {
  if (!grid || !dst || count == 0)
    return;
  size_t n = grid->values.size();
  if (count < n)
    n = count;
  if (n == 0)
    return;
  memcpy(dst, &grid->values[0], n * sizeof(float));
}

void mvv_corners_llh(MvvGrid* grid, double llh_out[8 * 3]) {
  if (!grid || !llh_out)
    return;
  const double lon1 = kLon0 + kLonExtent;
  const double lat1 = kLat0 + kLatExtent;
  const double h1 = kHeight0 + kHeightExtent;
  const double corner[8][3] = {
      {kLon0, kLat0, kHeight0}, {lon1, kLat0, kHeight0},
      {kLon0, lat1, kHeight0},  {lon1, lat1, kHeight0},
      {kLon0, kLat0, h1},       {lon1, kLat0, h1},
      {kLon0, lat1, h1},        {lon1, lat1, h1},
  };
  for (int i = 0; i < 8; ++i) {
    llh_out[i * 3 + 0] = corner[i][0];
    llh_out[i * 3 + 1] = corner[i][1];
    llh_out[i * 3 + 2] = corner[i][2];
  }
}

void mvv_close(MvvGrid* grid) {
  delete grid;
}

size_t mvv_value_count(const MvvGrid* grid) {
  if (!grid)
    return 0;
  return grid->values.size();
}

int mvv_time_index(const MvvGrid* grid) {
  if (!grid)
    return -1;
  return grid->time_index;
}

size_t mvv_set_time_index(MvvGrid* grid, int time_index) {
  if (!grid || time_index < 0)
    return 0;
  grid->time_index = time_index;
  Fill(grid);
  return 0;
}
