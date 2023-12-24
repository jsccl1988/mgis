// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_CONTENT_EXPORT_H
#define CONTENT_CONTENT_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(CONTENT_IMPLEMENTATION)
#define CONTENT_EXPORT __declspec(dllexport)
#else
#define CONTENT_EXPORT __declspec(dllimport)
#endif  // defined(CONTENT_IMPLEMENTATION)
#else
#define CONTENT_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define CONTENT_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // CONTENT_CONTENT_EXPORT_H