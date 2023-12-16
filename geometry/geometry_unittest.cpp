// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/logging.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "testing/test.h"

class TestGeometry : public ::testing::Test {
 protected:
  TestGeometry() {}
  ~TestGeometry() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestGeometry, test_log) {
  LOG(INFO) << __FUNCTION__ << "() logged";
}