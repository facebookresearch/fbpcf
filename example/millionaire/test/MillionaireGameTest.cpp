/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "../MillionaireGame.h" // @manual
#include "fbpcf/mpc/EmpTestUtil.h"
#include "fbpcf/system/CpuUtil.h"

namespace fbpcf {
TEST(MillionaireGame, AliceIsRicher) {
  if (!mpc::isTestable()) {
    GTEST_SKIP();
  }

  auto res = mpc::test<MillionaireGame<QueueIO>, int, bool>(2, 1);
  EXPECT_EQ(true, res.first);
  EXPECT_EQ(true, res.second);
}

TEST(MillionaireGame, BobIsRicher) {
  if (!system::isDrngSupported()) {
    GTEST_SKIP();
  }

  auto res = mpc::test<MillionaireGame<QueueIO>, int, bool>(1, 2);
  EXPECT_EQ(false, res.first);
  EXPECT_EQ(false, res.second);
}
} // namespace fbpcf
