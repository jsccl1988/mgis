// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef NATIVE_MVIEW_MAP_MVIEW_MAP_H_
#define NATIVE_MVIEW_MAP_MVIEW_MAP_H_

// C ABI for the 2D map. Camera and style layers stay in memory so headless
// tests can run without a GL surface. mvm_present draws into a child HWND
// when parent is non-NULL: MapLibre Native when mview_maplibre.dll is loaded,
// otherwise an OpenGL mercator frame.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <stdint.h>

#if defined(MVIEW_MAP_BUILD)
#define MVM_API __declspec(dllexport)
#else
#define MVM_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvmMap MvmMap;

// parent may be NULL. A child window is created on the first successful
// mvm_present, not during create, so headless tests stay off the GL path.
MVM_API MvmMap* mvm_create(HWND parent, int width, int height,
                           double pixel_ratio);

// kind 0 = mouse, 1 = wheel. action 1 = move, 2 = press, 3 = release.
typedef void (*MvmPointerFn)(void* user,
                             int kind,
                             int action,
                             double x,
                             double y,
                             double wheel_dx,
                             double wheel_dy);
MVM_API void mvm_set_pointer_callback(MvmMap* map, MvmPointerFn fn, void* user);

// 2 = MapLibre frame blitted, 1 = OpenGL mercator, 0 = no draw.
// Non-zero means pixels were presented and are not a single flat color.
MVM_API int mvm_present(MvmMap* map);
MVM_API unsigned int mvm_frame_token(MvmMap* map);
MVM_API int mvm_last_draw_calls(MvmMap* map);
MVM_API void mvm_resize(MvmMap* map, int width, int height, double pixel_ratio);
MVM_API void mvm_destroy(MvmMap* map);
MVM_API void mvm_set_notify(MvmMap* map, HWND target, UINT message);

MVM_API void mvm_set_style_url(MvmMap* map, const char* url);
MVM_API void mvm_set_camera(MvmMap* map, double lon, double lat, double zoom,
                            double bearing, double pitch);
MVM_API void mvm_get_camera(MvmMap* map, double* lon, double* lat, double* zoom,
                            double* bearing, double* pitch);
MVM_API void mvm_set_layer_visible(MvmMap* map, const char* layer_id,
                                   int visible);

// Writes a JSON array of {id,type,visible}. Returns the byte count excluding
// the trailing NUL, or -1 when map is NULL, the document exceeds 64 KB, or
// cap is too small. cap <= 0 or a NULL buffer reports the needed size.
MVM_API int mvm_list_layers(MvmMap* map, char* json, int cap);
MVM_API void mvm_add_geojson(MvmMap* map, const char* source_id,
                             const wchar_t* path);

// Stores a point query and, when a notify target was set, posts message
// with wParam = request_id. The stub has no rendered features.
MVM_API void mvm_query_point(MvmMap* map, double x, double y,
                             uint64_t request_id);

#ifdef __cplusplus
}
#endif

#endif  // NATIVE_MVIEW_MAP_MVIEW_MAP_H_
