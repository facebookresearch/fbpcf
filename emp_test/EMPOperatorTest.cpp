/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <emp-sh2pc/emp-sh2pc.h>
#include <folly/Random.h>
#include <gtest/gtest.h>

#include "folly/logging/xlog.h"
#include "../pcf/mpc/EmpTestUtil.h"
#include "../pcf/mpc/QueueIO.h"
#include "../pcf/system/CpuUtil.h"
#include "EMPOperator.hpp"
#include "EMPOperatorTestConfig.hpp"


namespace pcf {

class EMPOperatorTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!mpc::isTestable() || !system::isDrngSupported()) {
      GTEST_SKIP();
    }
  }
};

TEST_F(EMPOperatorTestFixture, AdditionTowPositives) {
  EMPOperatorTestConfig aliceConfig;
  aliceConfig = {EMPOperatorType::Addition, 20};
  EMPOperatorTestConfig bobConfig;
  bobConfig = {EMPOperatorType::Addition, 2000};

  auto [res1, res2] = pcf::mpc::
      test<EMPOperator<pcf::QueueIO, int64_t>, EMPOperatorTestConfig, int64_t>(
          aliceConfig, bobConfig);
  EXPECT_EQ(2020, res1);
  EXPECT_EQ(2020, res2);
}

TEST_F(EMPOperatorTestFixture, AdditionHybrid) {
  EMPOperatorTestConfig aliceConfig, bobConfig;
  aliceConfig = {EMPOperatorType::Addition, -2000};
  bobConfig = {EMPOperatorType::Addition, 20};
  auto [res1, res2] = pcf::mpc::
      test<EMPOperator<pcf::QueueIO, int64_t>, EMPOperatorTestConfig, int64_t>(
          aliceConfig, bobConfig);
  EXPECT_EQ(-1980, res1);
  EXPECT_EQ(-1980, res2);
}

TEST_F(EMPOperatorTestFixture, AdditionRandomNumbers) {
  int64_t num1 = folly::Random::rand32();
  int64_t num2 = folly::Random::rand32();
  int64_t ans_expected = num1 + num2;
  auto [res1, res2] = pcf::mpc::
      test<EMPOperator<pcf::QueueIO, int64_t>, EMPOperatorTestConfig, int64_t>(
          EMPOperatorTestConfig{EMPOperatorType::Addition, num1},
          EMPOperatorTestConfig{EMPOperatorType::Addition, num2});
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}
} // namespace pcf
