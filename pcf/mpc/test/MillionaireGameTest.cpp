#include <memory>

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "../../system/CpuUtil.h"
#include "../EmpTestUtil.h"
#include "./MillionaireGame.h"

namespace pcf {
TEST(MillionaireGame, AliceIsRicher) {
  if (!pcf::mpc::isTestable()) {
    GTEST_SKIP();
  }

  auto res = pcf::mpc::test<MillionaireGame<QueueIO>, int, bool>(2, 1);
  EXPECT_EQ(true, res.first);
  EXPECT_EQ(true, res.second);
}

TEST(MillionaireGame, BobIsRicher) {
  if (!pcf::system::isDrngSupported()) {
    GTEST_SKIP();
  }

  auto res = pcf::mpc::test<MillionaireGame<QueueIO>, int, bool>(1, 2);
  EXPECT_EQ(false, res.first);
  EXPECT_EQ(false, res.second);
}
} // namespace pcf
