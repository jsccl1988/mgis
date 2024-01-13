// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_3D_RENDERER_COMMON_H
#define GFX_3D_RENDERER_COMMON_H

#include <windef.h>
#include <windows.h>
#include <wingdi.h>
#include <wtypes.h>

#include <vector>

#include "base/base_config.h"
#include "base/basictypes.h"
#include "base/error.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "gfx/3d/renderer/render_export.h"

namespace gfx3d {
enum RHI3D { RHI3D_D3D, RHI3D_GL };
struct RenderOptions {};
}  // namespace gfx3d

#endif  // GFX_3D_RENDERER_COMMON_H