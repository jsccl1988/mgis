// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_MAP_SCENE_H_
#define CONTENT_BROWSER_MAP_SCENE_H_

#include <string>

#include "content/public/browser/scene_mode.h"

namespace content {

// Mode, camera host, and pick surface for the one scene view.
// This change ships FakeScene only. No renderer is linked here.
class Scene {
 public:
  virtual ~Scene() {}

  virtual SceneMode scene_mode() const = 0;

  // Primitive mark for the current mode, or empty when nothing is hit.
  // FakeScene ignores the pixel and selects by SceneMode.
  virtual std::string Pick(int x, int y) const = 0;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MAP_SCENE_H_
