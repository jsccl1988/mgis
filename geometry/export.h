// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef GEOMETRY_EXPORT_H
#define GEOMETRY_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(GEOMETRY_IMPLEMENTATION)
#define GEOMETRY_EXPORT __declspec(dllexport)
#else
#define GEOMETRY_EXPORT __declspec(dllimport)
#endif  // defined(GEOMETRY_IMPLEMENTATION)
#else
#define GEOMETRY_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define GEOMETRY_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // GEOMETRY_EXPORT_H