/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <random>
#include <variant>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/frontend/MPCTypes.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IntegerColumn.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
static std::vector<int32_t> deserializeAndRevealInt32(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    IColumnDefinition<schedulerId>& serializer) {
  auto scheduler = schedulerFactory.create();

  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));

  typename IColumnDefinition<schedulerId>::DeserializeType mpcValue =
      serializer.deserializeSharesToMPCType(serializedSecretShares, 0);
  typename frontend::MPCTypes<schedulerId>::Sec32Int visitedVal =
      std::get<typename frontend::MPCTypes<schedulerId>::Sec32Int>(mpcValue);

  std::vector<int64_t> rst64 = visitedVal.openToParty(0).getValue();
  std::vector<int32_t> rst(rst64.size());
  std::transform(
      rst64.begin(), rst64.end(), rst.begin(), [](int64_t val) { return val; });
  return rst;
}

template <int schedulerId>
static std::vector<std::vector<int32_t>> deserializeAndRevealInt32Vector(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    IColumnDefinition<schedulerId>& serializer) {
  auto scheduler = schedulerFactory.create();

  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));

  typename IColumnDefinition<schedulerId>::DeserializeType mpcValue =
      serializer.deserializeSharesToMPCType(serializedSecretShares, 0);
  std::vector<typename frontend::MPCTypes<schedulerId>::Sec32Int> visitedVal =
      std::get<std::vector<typename frontend::MPCTypes<schedulerId>::Sec32Int>>(
          mpcValue);

  std::vector<std::vector<int32_t>> rst(
      visitedVal.size(), std::vector<int32_t>(visitedVal[0].getBatchSize()));

  for (int i = 0; i < visitedVal.size(); i++) {
    std::vector<int64_t> rst64 = visitedVal[i].openToParty(0).getValue();
    std::transform(rst64.begin(), rst64.end(), rst[i].begin(), [](int64_t val) {
      return val;
    });
  }

  return rst;
}

TEST(SerializationTest, IntegerColumnTest) {
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto schedulerFactory0 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          0, *factories[0]);

  auto schedulerFactory1 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          1, *factories[1]);

  const size_t batchSize = 100;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> dist(
      std::numeric_limits<int32_t>().min(),
      std::numeric_limits<int32_t>().max());

  IntegerColumn<0, true, 32> serializer0("testColumnName");
  IntegerColumn<1, true, 32> serializer1("testColumnName");
  EXPECT_EQ(serializer0.getColumnSizeBytes(), 4);

  std::vector<std::vector<unsigned char>> bufs(
      batchSize, std::vector<unsigned char>(serializer0.getColumnSizeBytes()));

  std::vector<int32_t> vals(batchSize);

  for (int i = 0; i < 100; i++) {
    int32_t v = dist(e);

    vals[i] = v;
    serializer0.serializeColumnAsPlaintextBytes(&v, bufs[i].data());

    EXPECT_EQ(bufs[i][0], v & 255);
    EXPECT_EQ(bufs[i][1], (v >> 8) & 255);
    EXPECT_EQ(bufs[i][2], (v >> 16) & 255);
    EXPECT_EQ(bufs[i][3], (v >> 24) & 255);
  }

  auto future0 = std::async([&schedulerFactory0, &bufs, &serializer0]() {
    return deserializeAndRevealInt32<0>(schedulerFactory0, bufs, serializer0);
  });

  auto future1 = std::async([&schedulerFactory1, &serializer1]() {
    return deserializeAndRevealInt32<1>(
        schedulerFactory1,
        std::vector<std::vector<uint8_t>>(
            batchSize, std::vector<uint8_t>(serializer1.getColumnSizeBytes())),
        serializer1);
  });

  auto rst = future0.get();
  future1.get();
  testVectorEq(vals, rst);
}

TEST(SerializationTest, ArrayColumnTest) {
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto schedulerFactory0 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          0, *factories[0]);

  auto schedulerFactory1 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          1, *factories[1]);

  const size_t batchSize = 100;
  const size_t paddingSize = 4;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> dist(
      std::numeric_limits<int32_t>().min(),
      std::numeric_limits<int32_t>().max());

  FixedSizeArrayColumn<0, frontend::MPCTypes<0>::Sec32Int> serializer0(
      "testColumnName",
      std::make_unique<IntegerColumn<0, true, 32>>("testColumnName"),
      paddingSize);
  FixedSizeArrayColumn<1, frontend::MPCTypes<1>::Sec32Int> serializer1(
      "testColumnName",
      std::make_unique<IntegerColumn<1, true, 32>>("testColumnName"),
      paddingSize);
  EXPECT_EQ(serializer0.getColumnSizeBytes(), paddingSize * 4);

  std::vector<std::vector<uint8_t>> bufs(
      batchSize, std::vector<uint8_t>(serializer0.getColumnSizeBytes()));

  std::vector<std::vector<int32_t>> vals(
      paddingSize, std::vector<int32_t>(batchSize));

  for (int i = 0; i < batchSize; i++) {
    std::vector<int32_t> val(paddingSize);
    for (int j = 0; j < paddingSize; j++) {
      val[j] = dist(e);
      vals[j][i] = val[j];
    }

    serializer0.serializeColumnAsPlaintextBytes(val.data(), bufs[i].data());

    for (int j = 0; j < paddingSize; j++) {
      EXPECT_EQ(bufs[i][0 + j * sizeof(int32_t)], val[j] & 255);
      EXPECT_EQ(bufs[i][1 + j * sizeof(int32_t)], (val[j] >> 8) & 255);
      EXPECT_EQ(bufs[i][2 + j * sizeof(int32_t)], (val[j] >> 16) & 255);
      EXPECT_EQ(bufs[i][3 + j * sizeof(int32_t)], (val[j] >> 24) & 255);
    }
  }

  auto future0 = std::async([&schedulerFactory0, &bufs, &serializer0]() {
    return deserializeAndRevealInt32Vector<0>(
        schedulerFactory0, bufs, serializer0);
  });

  auto future1 = std::async([&schedulerFactory1, &serializer1]() {
    return deserializeAndRevealInt32Vector<1>(
        schedulerFactory1,
        std::vector<std::vector<uint8_t>>(
            batchSize, std::vector<uint8_t>(serializer1.getColumnSizeBytes())),
        serializer1);
  });

  auto rst = future0.get();
  future1.get();

  EXPECT_EQ(rst.size(), paddingSize);
  for (int j = 0; j < paddingSize; j++) {
    testVectorEq(vals[j], rst[j]);
  }
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
