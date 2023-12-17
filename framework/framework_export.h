// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef FRAMEWORK_FRAMEWORK_EXPORT_H
#define FRAMEWORK_FRAMEWORK_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(FRAMEWORK_IMPLEMENTATION)
#define FRAMEWORK_EXPORT __declspec(dllexport)
#else
#define FRAMEWORK_EXPORT __declspec(dllimport)
#endif  // defined(FRAMEWORK_IMPLEMENTATION)
#else
#define FRAMEWORK_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define FRAMEWORK_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // FRAMEWORK_FRAMEWORK_EXPORT_H