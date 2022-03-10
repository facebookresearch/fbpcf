/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/frontend/util.h"

namespace fbpcf::frontend {

TEST(TypeIndicatorTest, testIntegerTypes) {
  const int width = 31;
  EXPECT_EQ(width, Signed<width>::width);
  EXPECT_EQ(width, Unsigned<width>::width);

  EXPECT_TRUE(IsSigned<Signed<width>>::value);
  EXPECT_FALSE(IsSigned<Unsigned<width>>::value);

  EXPECT_TRUE(IsBatch<Batch<Signed<width>>>::value);
  EXPECT_TRUE(IsBatch<Batch<Unsigned<width>>>::value);

  EXPECT_FALSE(IsBatch<Signed<width>>::value);
  EXPECT_FALSE(IsBatch<Unsigned<width>>::value);
}

} // namespace fbpcf::frontend
