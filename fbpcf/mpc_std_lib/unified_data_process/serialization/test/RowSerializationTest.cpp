/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <algorithm>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/frontend/MPCTypes.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

#include "fbpcf/mpc_std_lib/unified_data_process/serialization/FixedSizeArrayColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IRowStructureDefinition.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IntegerColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/PackedBitFieldColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/RowStructureDefinition.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
std::unique_ptr<IRowStructureDefinition<schedulerId>> createRowDefinition() {
  auto columnDefs = std::make_unique<
      std::vector<typename std::unique_ptr<IColumnDefinition<schedulerId>>>>(0);

  columnDefs->push_back(
      std::make_unique<IntegerColumn<schedulerId, true, 32>>("int32Column"));
  columnDefs->push_back(
      std::make_unique<IntegerColumn<schedulerId, true, 64>>("int64Column"));
  columnDefs->push_back(
      std::make_unique<IntegerColumn<schedulerId, false, 32>>("uint32Column"));

  std::vector<std::string> bitColumnNames = {
      "boolColumn1", "boolColumn2", "boolColumn3", "boolColumn4"};
  columnDefs->push_back(std::make_unique<PackedBitFieldColumn<schedulerId>>(
      "packedBits", bitColumnNames));

  auto serializer = std::make_unique<RowStructureDefinition<schedulerId>>(
      std::move(columnDefs));
  return std::move(serializer);
}

template <int schedulerId>
std::unordered_map<
    std::string,
    typename IRowStructureDefinition<schedulerId>::InputColumnDataType>
deserializeAndRevealAllColumns(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    const std::unique_ptr<IRowStructureDefinition<schedulerId>>&
        rowDefinition) {
  auto scheduler = schedulerFactory.create();

  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));

  // The vector of vector of bytes would be passed into UDP, and the SecString
  // output will have less number of rows based on the intersection. We are
  // pretending there is no data filtered out and party 0 creates the SecString
  // as a private input.
  std::vector<std::vector<bool>> bitSharesTranspose(
      serializedSecretShares[0].size() * 8,
      std::vector<bool>(serializedSecretShares.size()));

  for (int i = 0; i < serializedSecretShares.size(); i++) {
    for (int j = 0; j < serializedSecretShares[i].size(); j++) {
      for (int k = 0; k < 8; k++) {
        bitSharesTranspose[j * 8 + k][i] =
            serializedSecretShares[i][j] >> k & 1;
      }
    }
  }

  frontend::BitString<true, schedulerId, true> udpOutput(bitSharesTranspose, 0);

  auto deserialization =
      rowDefinition.get()->deserializeUDPOutputIntoMPCTypes(udpOutput);

  std::unordered_map<
      std::string,
      typename IRowStructureDefinition<schedulerId>::InputColumnDataType>
      rst;

  std::vector<int64_t> int32Opened =
      std::get<typename frontend::MPCTypes<schedulerId>::Sec32Int>(
          deserialization.at("int32Column"))
          .openToParty(0)
          .getValue();
  std::vector<int32_t> int32Data(int32Opened.size());
  std::transform(
      int32Opened.begin(),
      int32Opened.end(),
      int32Data.begin(),
      [](int64_t data) { return data; });
  rst.emplace("int32Column", int32Data);

  std::vector<int64_t> int64Data =
      std::get<typename frontend::MPCTypes<schedulerId>::Sec64Int>(
          deserialization.at("int64Column"))
          .openToParty(0)
          .getValue();

  rst.emplace("int64Column", int64Data);

  std::vector<uint64_t> uint32Opened =
      std::get<typename frontend::MPCTypes<schedulerId>::SecUnsigned32Int>(
          deserialization.at("uint32Column"))
          .openToParty(0)
          .getValue();

  std::vector<uint32_t> uint32Data(uint32Opened.size());
  std::transform(
      uint32Opened.begin(),
      uint32Opened.end(),
      uint32Data.begin(),
      [](uint64_t data) { return data; });
  rst.emplace("uint32Column", uint32Data);

  std::vector<typename frontend::MPCTypes<schedulerId>::SecBool>
      packedBitsMPCValue = std::get<
          std::vector<typename frontend::MPCTypes<schedulerId>::SecBool>>(
          deserialization.at("packedBits"));

  rst.emplace("boolColumn1", packedBitsMPCValue[0].openToParty(0).getValue());
  rst.emplace("boolColumn2", packedBitsMPCValue[1].openToParty(0).getValue());
  rst.emplace("boolColumn3", packedBitsMPCValue[2].openToParty(0).getValue());
  rst.emplace("boolColumn4", packedBitsMPCValue[3].openToParty(0).getValue());

  return rst;
}

TEST(RowSerializationTest, RowWithMultipleColumnsTest) {
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
  std::uniform_int_distribution<> boolDist(0, 1);
  std::uniform_int_distribution<int64_t> uint32Dist(
      std::numeric_limits<uint32_t>().min(),
      std::numeric_limits<uint32_t>().max());
  std::uniform_int_distribution<int32_t> int32Dist(
      std::numeric_limits<int32_t>().min(),
      std::numeric_limits<int32_t>().max());
  std::uniform_int_distribution<int64_t> int64Dist(
      std::numeric_limits<int64_t>().min(),
      std::numeric_limits<int64_t>().max());

  std::unique_ptr<IRowStructureDefinition<0>> serializer0 =
      createRowDefinition<0>();
  std::unique_ptr<IRowStructureDefinition<1>> serializer1 =
      createRowDefinition<1>();

  EXPECT_EQ(serializer0->getRowSizeBytes(), 17);
  EXPECT_EQ(serializer1->getRowSizeBytes(), 17);

  std::vector<int32_t> int32Data(0);
  std::vector<int64_t> int64Data(0);
  std::vector<uint32_t> uint32Data(0);
  std::vector<bool> boolData1(0);
  std::vector<bool> boolData2(0);
  std::vector<bool> boolData3(0);
  std::vector<bool> boolData4(0);

  for (int i = 0; i < batchSize; i++) {
    int32Data.push_back(int32Dist(e));
    int64Data.push_back(int64Dist(e));
    uint32Data.push_back(uint32Dist(e));

    boolData1.push_back(boolDist(e));
    boolData2.push_back(boolDist(e));
    boolData3.push_back(boolDist(e));
    boolData4.push_back(boolDist(e));
  }

  std::unordered_map<
      std::string,
      IRowStructureDefinition<0>::InputColumnDataType>
      inputData{
          {"int32Column", int32Data},
          {"int64Column", int64Data},
          {"uint32Column", uint32Data},
          {"boolColumn1", boolData1},
          {"boolColumn2", boolData2},
          {"boolColumn3", boolData3},
          {"boolColumn4", boolData4}};

  auto serializedBytes =
      serializer0->serializeDataAsBytesForUDP(inputData, batchSize);

  auto future0 =
      std::async([&schedulerFactory0, &serializedBytes, &serializer0]() {
        return deserializeAndRevealAllColumns<0>(
            schedulerFactory0, serializedBytes, serializer0);
      });

  auto future1 = std::async([&schedulerFactory1, &serializer1]() {
    return deserializeAndRevealAllColumns<1>(
        schedulerFactory1,
        std::vector<std::vector<unsigned char>>(
            batchSize, std::vector<uint8_t>(serializer1->getRowSizeBytes())),
        serializer1);
  });

  auto rst = future0.get();
  future1.get();

  auto int32Rst = std::get<std::vector<int32_t>>(rst.at("int32Column"));
  auto int64Rst = std::get<std::vector<int64_t>>(rst.at("int64Column"));
  auto uint32Rst = std::get<std::vector<uint32_t>>(rst.at("uint32Column"));
  auto boolRst1 = std::get<std::vector<bool>>(rst.at("boolColumn1"));
  auto boolRst2 = std::get<std::vector<bool>>(rst.at("boolColumn2"));
  auto boolRst3 = std::get<std::vector<bool>>(rst.at("boolColumn3"));
  auto boolRst4 = std::get<std::vector<bool>>(rst.at("boolColumn4"));

  testVectorEq(int32Data, int32Rst);
  testVectorEq(int64Data, int64Rst);
  testVectorEq(uint32Data, uint32Rst);
  testVectorEq(boolData1, boolRst1);
  testVectorEq(boolData2, boolRst2);
  testVectorEq(boolData3, boolRst3);
  testVectorEq(boolData4, boolRst4);
}
} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
