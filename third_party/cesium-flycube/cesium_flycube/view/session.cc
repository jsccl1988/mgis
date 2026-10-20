// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/view/session.h"

#include "cesium_flycube/view/view_state_builder.h"

namespace cesium_flycube {

bool Session::CamerasShareOrigin(std::string* reason) const {
  return OriginPolicy::CamerasShareOrigin(georeference_, cameras_, reason);
}

std::vector<Cesium3DTilesSelection::ViewState> Session::BuildViewStates() const {
  return ViewStateBuilder::Build(georeference_, cameras_);
}

}  // namespace cesium_flycube
