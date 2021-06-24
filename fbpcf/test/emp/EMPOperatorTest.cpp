/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <functional>
#include <memory>

#include <emp-sh2pc/emp-sh2pc.h>
#include <folly/Random.h>
#include <gtest/gtest.h>

#include "folly/logging/xlog.h"

#include "EMPOperator.hpp"
#include "EMPOperatorTestConfig.hpp"
#include "fbpcf/mpc/EmpTestUtil.h"
#include "fbpcf/system/CpuUtil.h"

namespace fbpcf {

class EMPOperatorTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!mpc::isTestable() || !system::isDrngSupported()) {
      GTEST_SKIP();
    }
  }

  std::function<emp::Integer(emp::Integer, emp::Integer)> intAdd =
      [](const emp::Integer& x, const emp::Integer& y) { return x + y; };

  std::function<emp::Integer(emp::Integer, emp::Integer)> intMultiply =
      [](const emp::Integer& x, const emp::Integer& y) { return x * y; };

  std::function<emp::Integer(emp::Integer, emp::Integer)> intXor =
      [](const emp::Integer& x, const emp::Integer& y) { return x ^ y; };

  std::function<emp::Integer(emp::Integer, emp::Integer)> intCompare =
      [](const emp::Integer& x, const emp::Integer& y) {
        emp::Integer res{64, 0, emp::PUBLIC};
        if ((x > y).reveal<bool>()) {
          res = emp::Integer(64, 1, emp::PUBLIC);
        }
        return res;
      };

  std::function<emp::Integer(emp::Integer, emp::Integer)> intIsEqual =
      [](const emp::Integer& x, const emp::Integer& y) {
        emp::Integer res{64, 0, emp::PUBLIC};
        if ((x == y).reveal<bool>()) {
          res = emp::Integer(64, 1, emp::PUBLIC);
        }
        return res;
      };

  std::function<emp::Bit(emp::Bit, emp::Bit)> bitAnd =
      [](const emp::Bit& x, const emp::Bit& y) { return x & y; };

  std::function<emp::Bit(emp::Bit, emp::Bit)> bitXor =
      [](const emp::Bit& x, const emp::Bit& y) { return x ^ y; };
};

TEST_F(EMPOperatorTestFixture, IntAdditionTwoPositives) {
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intAdd, 20};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intAdd, 2000};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(2020, res1);
  EXPECT_EQ(2020, res2);
}

TEST_F(EMPOperatorTestFixture, IntAdditionHybrid) {
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intAdd, 20};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intAdd, -2000};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(-1980, res1);
  EXPECT_EQ(-1980, res2);
}

TEST_F(EMPOperatorTestFixture, IntAdditionRandomNumbers) {
  int64_t a = folly::Random::rand32();
  int64_t b = folly::Random::rand32();
  int64_t ans_expected = a + b;
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intAdd, a};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intAdd, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, IntXorRandomNumbers) {
  int64_t a = folly::Random::rand32();
  int64_t b = folly::Random::rand32();
  int64_t ans_expected = a ^ b;
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intXor, a};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intXor, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, IntMultiplicationRandomNumbers) {
  int64_t a = folly::Random::rand32(3037000500);
  int64_t b = folly::Random::rand32(3037000500);
  int64_t ans_expected = a * b;
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intMultiply, a};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intMultiply, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, IntCompareRandomNumbers) {
  int64_t a = folly::Random::rand32();
  int64_t b = folly::Random::rand32();
  int64_t ans_expected = a > b ? 1 : 0;
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intCompare, a};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intCompare, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, IntIsEqualRandomNumbers) {
  int64_t a = folly::Random::rand32();
  int64_t b = folly::Random::rand32();
  int64_t ans_expected = a == b ? 1 : 0;
  EMPOperatorTestConfig<int64_t, emp::Integer> aliceConfig{intIsEqual, a};
  EMPOperatorTestConfig<int64_t, emp::Integer> bobConfig{intIsEqual, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<
          int64_t,
          emp::Integer,
          EMPOperatorTestConfig<int64_t, emp::Integer>>,
      EMPOperatorTestConfig<int64_t, emp::Integer>,
      int64_t>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, BitAnd) {
  bool a = true;
  bool b = false;
  bool ans_expected = static_cast<bool>((a ? 1 : 0) & (b ? 1 : 0));
  EMPOperatorTestConfig<bool, emp::Bit> aliceConfig{bitAnd, a};
  EMPOperatorTestConfig<bool, emp::Bit> bobConfig{bitAnd, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<bool, emp::Bit, EMPOperatorTestConfig<bool, emp::Bit>>,
      EMPOperatorTestConfig<bool, emp::Bit>,
      bool>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

TEST_F(EMPOperatorTestFixture, BitXor) {
  bool a = true;
  bool b = false;
  bool ans_expected = a ^ b;
  EMPOperatorTestConfig<bool, emp::Bit> aliceConfig{bitXor, a};
  EMPOperatorTestConfig<bool, emp::Bit> bobConfig{bitXor, b};

  auto [res1, res2] = fbpcf::mpc::test<
      EMPOperator<bool, emp::Bit, EMPOperatorTestConfig<bool, emp::Bit>>,
      EMPOperatorTestConfig<bool, emp::Bit>,
      bool>(aliceConfig, bobConfig);
  EXPECT_EQ(ans_expected, res1);
  EXPECT_EQ(ans_expected, res2);
}

} // namespace fbpcf
