// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_PRODUCT_PRESENTERS_H_
#define CONTENT_PUBLIC_BROWSER_PRODUCT_PRESENTERS_H_

namespace content {

// Registers the product SceneImpl factory (MapLibre / UE→FlyCube). Call once
// before constructing GisContents in mgis. session_wire never
// calls this and keeps FakeScene.
void EnsureProductPresentersRegistered();

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_PRODUCT_PRESENTERS_H_
