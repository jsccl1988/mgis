// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// C API for one regular float grid (scalar or wind).
// This tree does not link VTK. Grids are synthetic. The header does not
// ray march and does not upload textures.

#ifndef NATIVE_MVIEW_VOLUME_MVIEW_VOLUME_H_
#define NATIVE_MVIEW_VOLUME_MVIEW_VOLUME_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default synthetic resolution on open. Resample may change it. */
#define MVV_SYNTHETIC_DIM 64

/* Opaque regular grid. Scalar: 1 component. Wind: U, V, W planar. */
typedef struct MvvGrid MvvGrid;

/* Opens a synthetic scalar grid. |path| and |variable| select the source
   name only; no file is read. |time_index| must be >= 0.
   Returns NULL and writes |err| on failure. */
MvvGrid* mvv_open_scalar(const wchar_t* path,
                         const char* variable,
                         int time_index,
                         char* err,
                         int err_cap);

/* Opens a synthetic wind grid (U, V, W) at |time_index|. */
MvvGrid* mvv_open_wind(const wchar_t* path,
                       int time_index,
                       char* err,
                       int err_cap);

/* Rebuilds the same procedural field at a new resolution.
   Each axis is clamped to 1..256. */
void mvv_resample(MvvGrid* grid, int nx, int ny, int nz);

/* Copies min(|count|, value count) floats into |dst|. */
void mvv_copy_f32(MvvGrid* grid, float* dst, size_t count);

/* Writes eight corners as longitude, latitude, height (degrees, degrees, meters). */
void mvv_corners_llh(MvvGrid* grid, double llh_out[8 * 3]);

void mvv_close(MvvGrid* grid);

/* Number of floats owned by the grid (nx * ny * nz * components). */
size_t mvv_value_count(const MvvGrid* grid);

/* Current time index, or -1 when |grid| is NULL. */
int mvv_time_index(const MvvGrid* grid);

/* Switches the procedural field to |time_index| in place.
   Does not allocate or copy a particle buffer.
   Returns the number of particle bytes copied (always 0).
   A negative index leaves the grid unchanged and still returns 0. */
size_t mvv_set_time_index(MvvGrid* grid, int time_index);

#ifdef __cplusplus
}
#endif

#endif  // NATIVE_MVIEW_VOLUME_MVIEW_VOLUME_H_
