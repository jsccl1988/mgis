// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/frame.h"

#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/TileContent.h>
#include <Cesium3DTilesSelection/ViewUpdateResult.h>
#include <CesiumUtility/CreditSystem.h>

namespace cesium_flycube {

TilesetFrame CollectTilesetFrame(
    const Cesium3DTilesSelection::ViewUpdateResult& update,
    CesiumUtility::CreditSystem& credits) {
  TilesetFrame frame;
  frame.tiles_to_render = update.tilesToRenderThisFrame.size();
  for (const Cesium3DTilesSelection::Tile::ConstPointer& tile :
       update.tilesToRenderThisFrame) {
    const Cesium3DTilesSelection::TileRenderContent* content =
        tile->getContent().getRenderContent();
    if (content == nullptr || content->getRenderResources() == nullptr) {
      continue;
    }
    const auto* mesh =
        static_cast<const TileMesh*>(content->getRenderResources());
    frame.meshes.push_back(*mesh);
    frame.vertex_count += mesh->ecef_positions.size();
    frame.index_count += mesh->indices.size();
  }
  frame.meshes_ready = frame.meshes.size();

  const CesiumUtility::CreditsSnapshot& snapshot = credits.getSnapshot();
  frame.credits.reserve(snapshot.currentCredits.size());
  for (const CesiumUtility::Credit credit : snapshot.currentCredits) {
    frame.credits.push_back(credits.getHtml(credit));
  }
  return frame;
}

}  // namespace cesium_flycube
