// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "base/logging.h"
#include "core/sdb/ogr/gdal.hpp"
#include "testing/test.h"

class TestSDB : public ::testing::Test {
 protected:
  TestSDB() {}
  ~TestSDB() override {}
  void SetUp() override {}
  void TearDown() override {}

  core::Dataset dataset_{"MEM", "ForTest"};
};

TEST_F(TestSDB, test_create_layer) {
  core::Layer layer(dataset_, "Polygon", wkbPolygon);
  layer.add_field("省", OFTString, 20);
  layer.add_field("面积", OFTReal, 20);
  layer.add_field("周长", OFTReal, 20);

  std::string wkt{"POLYGON((500 500, 1000 500, 1000 1000, 500 1000, 500 500))"};
  std::unique_ptr<OGRPolygon> geometry = std::make_unique<OGRPolygon>();
  auto* buffer = wkt.data();
  geometry.get()->importFromWkt(&buffer);

  layer.start_transaction();
  core::Feature feature(layer, std::move(geometry));
  feature.set_field(0, "上海");
  feature.set_field(1, 1000);
  feature.set_field(2, 2000);
  layer.commit_transaction();

  LOG(INFO) << __FUNCTION__ << layer.get().GetFeatureCount();
}