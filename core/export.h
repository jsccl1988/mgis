// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef CORE_EXPORT_H
#define CORE_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(CORE_IMPLEMENTATION)
#define CORE_EXPORT __declspec(dllexport)
#else
#define CORE_EXPORT __declspec(dllimport)
#endif  // defined(CORE_IMPLEMENTATION)
#else
#define CORE_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define CORE_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // CORE_EXPORT_H