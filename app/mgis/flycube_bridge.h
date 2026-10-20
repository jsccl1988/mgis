// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_FLYCUBE_BRIDGE_H_
#define APP_MGIS_FLYCUBE_BRIDGE_H_

namespace mgis {

// Registers the in-process globe as the FlyCube view backend.
// Owned by the workbench. The shell incubator does not call this.
void InstallFlyCubeBackend();

}  // namespace mgis

#endif  // APP_MGIS_FLYCUBE_BRIDGE_H_
