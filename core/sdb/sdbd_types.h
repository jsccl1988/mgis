// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef SDB_SDBD_TYPES_H
#define SDB_SDBD_TYPES_H

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace core {
namespace sdb {

// Contract keys from smartgis/docs/superpowers/04-data-sdbd.md §3.2.

struct BBox {
  double min_x = 0;
  double min_y = 0;
  double max_x = 0;
  double max_y = 0;

  bool Intersects(const BBox& other) const {
    return min_x <= other.max_x && max_x >= other.min_x &&
           min_y <= other.max_y && max_y >= other.min_y;
  }
};

struct FieldInfo {
  std::string name;
  std::string type;
};

struct LayerInfo {
  std::string name;
  std::string crs;
  std::string geom_type;
  std::string geom_column;
  BBox extent;
  std::vector<FieldInfo> fields;
};

struct OpenRequest {
  std::string layer;
  std::vector<std::string> columns;
  BBox bbox;
  std::string predicate{"intersects"};
  std::string crs;
  int limit = 10000;
};

struct Feature {
  std::string id;
  std::string geom_wkt;
  std::map<std::string, std::string> attrs;
};

class FeatureSet {
 public:
  std::string layer;
  std::string crs;
  int count = 0;
  int offset = 0;
  std::vector<Feature> features;

  void MoveFirst();
  bool MoveNext();
  bool IsEnd() const;
  const Feature* GetFeature() const;
  std::size_t GetFeatureCount() const { return features.size(); }

 private:
  std::size_t index_ = 0;
};

struct SdbdResult {
  bool ok = false;
  int status = 0;
  std::string body;
  std::string error;
  std::string message;
};

inline constexpr char kDefaultSdbdBase[] = "http://127.0.0.1:8021";
inline constexpr char kApiPrefix[] = "/sdbd/api/v1";
inline constexpr int kLimitHardCap = 100000;
inline constexpr int kFetchDefault = 1000;
inline constexpr int kFetchMax = 10000;
inline constexpr char kDemoLayer[] = "t_rtree_demo";
inline constexpr char kDemoCrs[] = "EPSG:4326";

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_TYPES_H
