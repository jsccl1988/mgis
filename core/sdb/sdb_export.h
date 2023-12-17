// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef SDB_SDB_EXPORT_H
#define SDB_SDB_EXPORT_H

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if defined(SDB_IMPLEMENTATION)
#define SDB_EXPORT __declspec(dllexport)
#else
#define SDB_EXPORT __declspec(dllimport)
#endif  // defined(SDB_IMPLEMENTATION)
#else
#define SDB_EXPORT __attribute__((visibility("default")))
#endif  // defined(WIN32)
#else
#define SDB_EXPORT
#endif  // defined(COMPONENT_BUILD)

#endif  // SDB_SDB_EXPORT_H