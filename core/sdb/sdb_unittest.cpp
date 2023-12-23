// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "base/logging.h"
#include "ogrsf_frmts.h"
#include "testing/test.h"

class TestSDB : public ::testing::Test {
 protected:
  TestSDB() {}
  ~TestSDB() override {}
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestSDB, test_log) {}