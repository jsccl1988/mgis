// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef VISTA_VISTA_EXPORT_H_
#define VISTA_VISTA_EXPORT_H_

#if defined(VISTA_MAP_IMPLEMENTATION) || defined(VISTA_WORLD_IMPLEMENTATION)
#define VISTA_EXPORT __declspec(dllexport)
#else
#define VISTA_EXPORT __declspec(dllimport)
#endif

#endif  // VISTA_VISTA_EXPORT_H_
