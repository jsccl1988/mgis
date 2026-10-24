// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/viewport/map/scene_factory.h"

#include "content/browser/viewport/map/fake_scene.h"

namespace content {
namespace {

SceneFactoryFn g_scene_factory = NULL;

}  // namespace

void SetSceneFactory(SceneFactoryFn factory) {
  g_scene_factory = factory;
}

Scene* CreateScene() {
  if (g_scene_factory)
    return g_scene_factory();
  return new FakeScene();
}

}  // namespace content
