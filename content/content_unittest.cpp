// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestContent : public ::testing::Test {
 protected:
  TestContent() {}
  ~TestContent() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestContent, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }