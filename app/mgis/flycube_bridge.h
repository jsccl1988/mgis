// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_FLYCUBE_BRIDGE_H_
#define APP_MGIS_FLYCUBE_BRIDGE_H_

namespace mgis {

// Scene children attach through GisContents::AttachView. GlobeView stays
// available for render checks and is not registered as a view0 backend.
void InstallFlyCubeBackend();

}  // namespace mgis

#endif  // APP_MGIS_FLYCUBE_BRIDGE_H_
