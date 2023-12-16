// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/init.h"
#include "base/logging.h"
#include "testing/test.h"

int main(int argc, char* argv[]) {
  base::Initialize(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  RUN_ALL_TESTS();
  base::Deinitialize();

  return 0;
}