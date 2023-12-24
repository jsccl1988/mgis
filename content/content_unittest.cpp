// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestFramework : public ::testing::Test {
 protected:
  TestFramework() {}
  ~TestFramework() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestFramework, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }