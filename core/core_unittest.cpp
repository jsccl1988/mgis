// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestCore : public ::testing::Test {
 protected:
  TestCore() {}
  ~TestCore() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestCore, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }