// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: matches smartgis `include/sg/clients/mapd_client.h`.
// Health() is a thin extra (GET /mapd/api/v1/health) used by link-spatial M1.

#pragma once

#include "http_transport.h"
#include "query_request.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace sg {

enum class TileKind : std::uint8_t {
  kRaster = 0,
  kVector = 1,
  kTerrain = 2,
  kSatellite = 3,
};

struct MapdResult {
  bool ok = false;
  int status = 0;
  std::string body;
  std::string error;
};

struct Capabilities {
  std::string crs;
  std::string data_version;
  bool path_q_features = false;
  bool path_q_analysis = false;
  bool postgis_configured = false;
  std::string primary_store;
  bool path_r_raster = false;
  bool path_r_vector = false;
  std::vector<std::string> query_layers;
};

class DualPathQueue {
 public:
  enum class Kind : std::uint8_t { kQ = 0, kR = 1 };

  struct Item {
    Kind kind = Kind::kQ;
    std::string label;
  };

  void Enqueue(Kind kind, std::string label);
  bool TryDequeueR(Item* out);
  bool TryDequeueQ(Item* out);
  std::size_t pending_q() const;
  std::size_t pending_r() const;

 private:
  mutable std::mutex mu_;
  std::queue<Item> q_;
  std::queue<Item> r_;
};

class MapdClient {
 public:
  MapdClient(std::string base_url, std::shared_ptr<HttpTransport> transport);

  const std::string& base_url() const { return base_url_; }
  DualPathQueue& queues() { return queues_; }
  const DualPathQueue& queues() const { return queues_; }

  MapdResult Health();
  MapdResult GetStyle();
  MapdResult GetCapabilities();
  MapdResult Query(const QueryRequest& req);
  std::string TileUrl(TileKind kind, int z, int x, int y) const;

  static bool ParseCapabilities(const std::string& json, Capabilities* out);

 private:
  MapdResult Send(HttpRequest req);

  std::string base_url_;
  std::shared_ptr<HttpTransport> transport_;
  DualPathQueue queues_;
};

}  // namespace sg
