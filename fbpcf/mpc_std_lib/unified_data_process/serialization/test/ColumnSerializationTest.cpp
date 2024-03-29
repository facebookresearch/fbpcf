/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
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

#include "fbpcf/mpc_std_lib/unified_data_process/serialization/FixedSizeArrayColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IntegerColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/PackedBitFieldColumn.h"

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

template <int schedulerId>
static std::vector<std::vector<bool>> deserializeAndRevealPackedBits(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    IColumnDefinition<schedulerId>& serializer) {
  auto scheduler = schedulerFactory.create();

  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));

  typename IColumnDefinition<schedulerId>::DeserializeType mpcValue =
      serializer.deserializeSharesToMPCType(serializedSecretShares, 0);
  std::vector<typename frontend::MPCTypes<schedulerId>::SecBool> visitedVal =
      std::get<std::vector<typename frontend::MPCTypes<schedulerId>::SecBool>>(
          mpcValue);

  std::vector<std::vector<bool>> rst(
      visitedVal.size(), std::vector<bool>(visitedVal[0].getBatchSize()));

  for (int i = 0; i < visitedVal.size(); i++) {
    rst[i] = visitedVal[i].openToParty(0).getValue();
  }

  return rst;
}

TEST(ColumnSerializationTest, IntegerColumnTest) {
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
  }

  serializer0.serializeColumnAsPlaintextBytes(vals, bufs, 0);

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(bufs[i][0], vals[i] & 255);
    EXPECT_EQ(bufs[i][1], (vals[i] >> 8) & 255);
    EXPECT_EQ(bufs[i][2], (vals[i] >> 16) & 255);
    EXPECT_EQ(bufs[i][3], (vals[i] >> 24) & 255);
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

TEST(ColumnSerializationTest, ArrayColumnTest) {
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

  FixedSizeArrayColumn<0, frontend::MPCTypes<0>::Sec32Int, int32_t> serializer0(
      "testColumnName", paddingSize);
  FixedSizeArrayColumn<1, frontend::MPCTypes<1>::Sec32Int, int32_t> serializer1(
      "testColumnName", paddingSize);
  EXPECT_EQ(serializer0.getColumnSizeBytes(), paddingSize * 4);

  std::vector<std::vector<uint8_t>> bufs(
      batchSize, std::vector<uint8_t>(serializer0.getColumnSizeBytes()));

  std::vector<std::vector<int32_t>> vals(
      batchSize, std::vector<int32_t>(paddingSize));

  std::vector<std::vector<int32_t>> expected(
      paddingSize, std::vector<int32_t>(batchSize));

  for (int i = 0; i < batchSize; i++) {
    for (int j = 0; j < paddingSize; j++) {
      vals[i][j] = dist(e);
      expected[j][i] = vals[i][j];
    }
  }

  serializer0.serializeColumnAsPlaintextBytes(vals, bufs, 0);

  for (int i = 0; i < batchSize; i++) {
    for (int j = 0; j < paddingSize; j++) {
      EXPECT_EQ(bufs[i][0 + j * sizeof(int32_t)], vals[i][j] & 255);
      EXPECT_EQ(bufs[i][1 + j * sizeof(int32_t)], (vals[i][j] >> 8) & 255);
      EXPECT_EQ(bufs[i][2 + j * sizeof(int32_t)], (vals[i][j] >> 16) & 255);
      EXPECT_EQ(bufs[i][3 + j * sizeof(int32_t)], (vals[i][j] >> 24) & 255);
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
    testVectorEq(expected[j], rst[j]);
  }
}

TEST(ColumnSerializationTest, PackedBitFieldColumnTest) {
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto schedulerFactory0 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          0, *factories[0]);

  auto schedulerFactory1 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          1, *factories[1]);

  const size_t batchSize = 100;
  const size_t numBits = 7;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<> dist(0, 1);

  PackedBitFieldColumn<0> serializer0(
      "testColumnName", std::vector<std::string>(7, "testColumnName"));
  PackedBitFieldColumn<1> serializer1(
      "testColumnName", std::vector<std::string>(7, "testColumnName"));
  EXPECT_EQ(serializer0.getColumnSizeBytes(), 1);

  std::vector<std::vector<uint8_t>> bufs(
      batchSize, std::vector<uint8_t>(serializer0.getColumnSizeBytes()));

  std::vector<std::vector<bool>> vals(batchSize, std::vector<bool>(numBits));
  std::vector<std::vector<bool>> expected(
      numBits, std::vector<bool>(batchSize));

  for (int i = 0; i < batchSize; i++) {
    for (int j = 0; j < numBits; j++) {
      vals[i][j] = dist(e);
      expected[j][i] = vals[i][j];
    }
  }

  serializer0.serializeColumnAsPlaintextBytes(vals, bufs, 0);
  for (int i = 0; i < batchSize; i++) {
    for (int j = 0; j < numBits; j++) {
      EXPECT_EQ((bufs[i][0] >> j) & 1, vals[i][j]);
    }
  }

  auto future0 = std::async([&schedulerFactory0, &bufs, &serializer0]() {
    return deserializeAndRevealPackedBits<0>(
        schedulerFactory0, bufs, serializer0);
  });

  auto future1 = std::async([&schedulerFactory1, &serializer1]() {
    return deserializeAndRevealPackedBits<1>(
        schedulerFactory1,
        std::vector<std::vector<uint8_t>>(
            batchSize, std::vector<uint8_t>(serializer1.getColumnSizeBytes())),
        serializer1);
  });

  auto rst = future0.get();
  future1.get();

  EXPECT_EQ(rst.size(), numBits);
  for (int j = 0; j < numBits; j++) {
    testVectorEq(expected[j], rst[j]);
  }
}

TEST(erializationTest, ColumnTypeTest) {
  using ColType = IColumnDefinition<0>::SupportedColumnTypes;
  std::unique_ptr<IColumnDefinition<0>> col0 =
      std::make_unique<IntegerColumn<0, true, 32>>("col0");
  EXPECT_EQ(col0->getColumnType(), ColType::Int32);

  std::unique_ptr<IColumnDefinition<0>> col1 =
      std::make_unique<IntegerColumn<0, true, 64>>("col1");
  EXPECT_EQ(col1->getColumnType(), ColType::Int64);

  std::unique_ptr<IColumnDefinition<0>> col2 =
      std::make_unique<IntegerColumn<0, false, 32>>("col2");
  EXPECT_EQ(col2->getColumnType(), ColType::UInt32);

  std::vector<std::string> names{"bool1", "bool2"};
  std::unique_ptr<IColumnDefinition<0>> col3 =
      std::make_unique<PackedBitFieldColumn<0>>("col3", names);
  EXPECT_EQ(col3->getColumnType(), ColType::PackedBitField);

  std::unique_ptr<IColumnDefinition<0>> col4 = std::make_unique<
      FixedSizeArrayColumn<0, frontend::MPCTypes<0>::Sec32Int, int32_t>>(
      "col4", 4);
  EXPECT_EQ(col4->getColumnType(), ColType::Int32Vec);

  std::unique_ptr<IColumnDefinition<0>> col5 = std::make_unique<
      FixedSizeArrayColumn<0, frontend::MPCTypes<0>::Sec64Int, int64_t>>(
      "col4", 4);
  EXPECT_EQ(col5->getColumnType(), ColType::Int64Vec);

  std::unique_ptr<IColumnDefinition<0>> col6 =
      std::make_unique<FixedSizeArrayColumn<
          0,
          frontend::MPCTypes<0>::SecUnsigned32Int,
          uint32_t>>("col4", 4);
  EXPECT_EQ(col6->getColumnType(), ColType::UInt32Vec);
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
