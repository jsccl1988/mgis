// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestGUI : public ::testing::Test {
 protected:
  TestGUI() {}
  ~TestGUI() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestGUI, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }