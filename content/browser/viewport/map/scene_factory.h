// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_MAP_SCENE_FACTORY_H_
#define CONTENT_BROWSER_VIEWPORT_MAP_SCENE_FACTORY_H_

namespace content {

class Scene;

// Product shells may replace the default FakeScene factory before constructing
// GisContents. session_wire never registers a product factory.
typedef Scene* (*SceneFactoryFn)();

void SetSceneFactory(SceneFactoryFn factory);
Scene* CreateScene();

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_MAP_SCENE_FACTORY_H_
