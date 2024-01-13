// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "base/logging.h"
#include "testing/test.h"

class TestRender3D : public ::testing::Test {
 protected:
  TestRender3D() {}
  ~TestRender3D() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestRender3D, test_log) { LOG(INFO) << __FUNCTION__ << "() logged"; }