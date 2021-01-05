/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../../pcf/mpc/EmpTestUtil.h"
#include "../../pcf/mpc/EmpVector.h"
#include "../../pcf/mpc/QueueIO.h"
#include "../common/GroupedLiftMetrics.h"
#include "MetricsMapper.h"

namespace private_lift {
class MetricsMapperTest
    : public pcf::
          EmpGame<pcf::QueueIO, GroupedLiftMetrics, GroupedLiftMetrics> {
 public:
  MetricsMapperTest(std::unique_ptr<pcf::QueueIO> io, pcf::Party role)
      : EmpGame(std::move(io), role) {}

  GroupedLiftMetrics play(const GroupedLiftMetrics& input) override {
    // serialize GroupedLiftMetrics to EmpVector
    auto empVector = mapGroupedLiftMetricsToEmpVector(input);

    // For each field, Alice ^ Bob
    auto resVector =
        empVector.map([](const auto& x, const auto& y) { return x ^ y; });

    // Reveal results to PUBLIC
    auto revealedVector = pcf::functional::map<emp::Integer, int64_t>(
        resVector,
        [](emp::Integer i) { return i.reveal<int64_t>(emp::PUBLIC); });

    // deserialize vector to GroupedLiftMetrics
    return mapVectorToGroupedLiftMetrics(revealedVector);
  }
};

TEST(MetricsMapperTest, TestXorMetrics) {
  GroupedLiftMetrics expectedMetrics{
      LiftMetrics{0, 0, 0, 0, 0, 0, 0, 0},
      std::vector<LiftMetrics>{LiftMetrics{0, 0, 0, 0, 0, 0, 0, 0}}};

  GroupedLiftMetrics inputMetrics{
      LiftMetrics{1, 2, 3, 4, 5, 6, 7, 8},
      std::vector<LiftMetrics>{LiftMetrics{1, 2, 3, 4, 5, 6, 7, 8}}};

  auto res =
      pcf::mpc::test<MetricsMapperTest, GroupedLiftMetrics, GroupedLiftMetrics>(
          inputMetrics, inputMetrics);

  EXPECT_EQ(expectedMetrics, res.first);
  EXPECT_EQ(expectedMetrics, res.second);
}
} // namespace private_lift
