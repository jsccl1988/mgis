// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_EXPORT_H
#define BASE_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)
#else
#define BASE_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define BASE_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // BASE_EXPORT_H