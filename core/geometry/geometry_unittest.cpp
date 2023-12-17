// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "core/geometry/geometry.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestGeometry : public ::testing::Test {
 protected:
  TestGeometry() {}
  ~TestGeometry() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestGeometry, test_geos) {
  using namespace core;
  GeometryFactory::Ptr factory = GeometryFactory::create();
  WKTReader reader(*factory);

  std::string wkt_a("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
  std::string wkt_b("POLYGON((5 5, 15 5, 15 15, 5 15, 5 5))");

  std::unique_ptr<Geometry> geom_a(reader.read(wkt_a));
  std::unique_ptr<Geometry> geom_b(reader.read(wkt_b));

  std::unique_ptr<Geometry> inter = geom_a->intersection(geom_b.get());

  WKTWriter writer;
  writer.setTrim(true);
  std::string inter_wkt = writer.write(inter.get());

  LOG(INFO) << "Geometry A:         " << wkt_a << std::endl;
  LOG(INFO) << "Geometry B:         " << wkt_b << std::endl;
  LOG(INFO) << "Intersection(A, B): " << inter_wkt << std::endl;
}