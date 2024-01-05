// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTRA_VIEWS_EXPORT_H_
#define EXTRA_VIEWS_EXPORT_H_

// Defines CONTROL_EXPORT so that functionality implemented by the Views module
// can be exported to consumers.

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(EXTRA_VIEWS_IMPLEMENTATION)
#define EXTRA_EXPORT __declspec(dllexport)
#else
#define EXTRA_EXPORT __declspec(dllimport)
#endif  // defined(EXTRA_VIEWS_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(EXTRA_VIEWS_IMPLEMENTATION)
#define EXTRA_EXPORT __attribute__((visibility("default")))
#else
#define EXTRA_EXPORT
#endif
#endif

#else  // defined(COMPONENT_BUILD)
#define EXTRA_EXPORT
#endif

#endif  // EXTRA_VIEWS_EXPORT_H_
