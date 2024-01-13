// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef GFX_2D_RENDER_RENDER_EXPORT_H
#define GFX_2D_RENDER_RENDER_EXPORT_H

#include "base/base_config.h"

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(GFX_2D_RENDER_DEVICE_IMPLEMENTATION)
#define GFX_2D_RENDERER_EXPORT __declspec(dllexport)
#else
#define GFX_2D_RENDERER_EXPORT __declspec(dllimport)
#endif  // defined(GFX_2D_RENDER_DEVICE_IMPLEMENTATION)
#else
#define GFX_2D_RENDERER_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define GFX_2D_RENDERER_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // GFX_2D_RENDER_RENDER_EXPORT_H