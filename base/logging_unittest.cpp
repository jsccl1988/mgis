// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/logging.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "testing/test.h"

class TestLogging : public ::testing::Test {
 protected:
  TestLogging() {}
  ~TestLogging() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestLogging, test_log) {
  LOG(INFO) << __FUNCTION__ << "() logged";
}