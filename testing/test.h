// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_TESTING_TEST
#define BASE_TESTING_TEST

#include <gtest/gtest.h>

#include <random>
#include <stdexcept>

#include "base/logging.h"

#define _(...) __VA_ARGS__

#define ASSERT_THROWS_WITH_INTERNAL(matcher, ...) \
  do {                                            \
    try {                                         \
      static_cast<void>(__VA_ARGS__);             \
    } catch (const std::runtime_error& e) {       \
      ASSERT_STREQ(e.what(), matcher);            \
    } catch (...) {                               \
    }                                             \
  } while (0)

#define ASSERT_THROWS_WITH(expr, matcher) \
  ASSERT_THROWS_WITH_INTERNAL(matcher, expr)

#define TYPE_CHECK_EQ(lhs, rhs) \
  static_assert(std::is_same<lhs, rhs>::value, #lhs " == " #rhs)

#define TYPE_CHECK_NEQ(lhs, rhs) \
  static_assert(!std::is_same<lhs, rhs>::value, #lhs " == " #rhs)

#endif  // BASE_TESTING_TEST