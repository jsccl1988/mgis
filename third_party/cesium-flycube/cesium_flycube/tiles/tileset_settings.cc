// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/tileset_settings.h"

namespace cesium_flycube {

void ApplyTilesetSettings(const TilesetSettings& settings,
                          Cesium3DTilesSelection::TilesetOptions* options) {
  if (options == nullptr) {
    return;
  }
  options->maximumScreenSpaceError = settings.maximum_screen_space_error;
  options->maximumSimultaneousTileLoads =
      settings.maximum_simultaneous_tile_loads;
  options->preloadAncestors = settings.preload_ancestors;
  options->preloadSiblings = settings.preload_siblings;
  options->loadingDescendantLimit = settings.loading_descendant_limit;
  options->forbidHoles = settings.forbid_holes;
  options->enableFrustumCulling = settings.enable_frustum_culling;
  options->enableOcclusionCulling = settings.enable_occlusion_culling;
  options->delayRefinementForOcclusion =
      settings.delay_refinement_for_occlusion;
  options->enableFogCulling = settings.enable_fog_culling;
  options->enforceCulledScreenSpaceError =
      settings.enforce_culled_screen_space_error;
  options->culledScreenSpaceError = settings.culled_screen_space_error;
  options->maximumCachedBytes = settings.maximum_cached_bytes;
  options->renderTilesUnderCamera = settings.render_tiles_under_camera;
  options->enableLodTransitionPeriod = settings.enable_lod_transition_period;
  options->lodTransitionLength = settings.lod_transition_length;
  options->showCreditsOnScreen = settings.show_credits_on_screen;
}

}  // namespace cesium_flycube
