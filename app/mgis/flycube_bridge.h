// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_FLYCUBE_BRIDGE_H_
#define APP_MGIS_FLYCUBE_BRIDGE_H_

namespace mgis {

// No longer registers a view0 backend. Scene children attach through
// GisContents::AttachView. GlobeView itself stays available for render checks.
void InstallFlyCubeBackend();

}  // namespace mgis

#endif  // APP_MGIS_FLYCUBE_BRIDGE_H_
