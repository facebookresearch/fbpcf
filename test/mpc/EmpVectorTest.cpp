/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <gtest/gtest.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/exception/PcfException.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../../pcf/mpc/EmpTestUtil.h"
#include "../../pcf/mpc/EmpVector.h"
#include "../../pcf/mpc/QueueIO.h"
#include "../../pcf/system/CpuUtil.h"

namespace pcf {
class EmpVectorTest
    : public EmpGame<QueueIO, std::vector<int64_t>, std::vector<int64_t>> {
 public:
  EmpVectorTest(std::unique_ptr<QueueIO> io, Party party)
      : EmpGame(std::move(io), party) {}

  std::vector<int64_t> play(const std::vector<int64_t>& input) override {
    EmpVector<emp::Integer> v;

    v.add(input);

    auto res = v.map([](emp::Integer x, emp::Integer y) { return x ^ y; });
    return pcf::functional::map<emp::Integer, int64_t>(
        res, [](emp::Integer i) { return i.reveal<int64_t>(emp::PUBLIC); });
  }
};

TEST(EmpVectorTest, AddIntegerWithException) {
  EmpVector<emp::Integer> v;
  EXPECT_THROW(v.add(true), PcfException);
}

TEST(EmpVectorTest, AddBitWithException) {
  EmpVector<emp::Bit> v;
  EXPECT_THROW(v.add(1), PcfException);
}

TEST(EmpVectorTest, TestMap) {
  if (!pcf::mpc::isTestable()) {
    GTEST_SKIP();
  }

  std::vector<int64_t> expectedRes{0, 0, 0};

  auto res =
      pcf::mpc::testGame<EmpVectorTest, std::vector<int64_t>, std::vector<int64_t>>(
          std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{1, 2, 3});

  EXPECT_EQ(expectedRes, res.first);
  EXPECT_EQ(expectedRes, res.second);
}
} // namespace pcf
