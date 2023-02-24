/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <string>

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
std::unique_ptr<IRowStructureDefinition<schedulerId>> createRowDefinition(
    size_t paddingSize) {
  using ColType = typename IColumnDefinition<schedulerId>::SupportedColumnTypes;
  std::map<std::string, ColType> colDefs{
      {"int32Column", ColType::Int32},
      {"int64Column", ColType::Int64},
      {"uint32Column", ColType::UInt32},
      {"int32VecColumn", ColType::Int32Vec},
      {"int64VecColumn", ColType::Int64Vec},
      {"uint32VecColumn", ColType::UInt32Vec},
      {"boolColumn1", ColType::Bit},
      {"boolColumn2", ColType::Bit},
      {"boolColumn3", ColType::Bit},
      {"boolColumn4", ColType::Bit},
  };
  auto serializer = std::make_unique<RowStructureDefinition<schedulerId>>(
      colDefs, paddingSize);
  return std::move(serializer);
}

using RevealedColumnType = std::variant<
    std::vector<bool>,
    std::vector<uint32_t>,
    std::vector<int32_t>,
    std::vector<int64_t>,
    std::vector<std::vector<bool>>,
    std::vector<std::vector<uint32_t>>,
    std::vector<std::vector<int32_t>>,
    std::vector<std::vector<int64_t>>>;

template <int schedulerId>
typename frontend::BitString<true, schedulerId, true> deserializeIntoSecString(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares) {
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

  return udpOutput;
}

template <int schedulerId>
std::unordered_map<std::string, RevealedColumnType>
deserializeAndRevealAllColumns(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    const std::unique_ptr<IRowStructureDefinition<schedulerId>>&
        rowDefinition) {
  auto udpOutput = deserializeIntoSecString<schedulerId>(
      schedulerFactory, serializedSecretShares);
  auto deserialization =
      rowDefinition.get()->deserializeUDPOutputIntoMPCTypes(udpOutput);

  std::unordered_map<std::string, RevealedColumnType> rst;

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

  std::vector<typename frontend::MPCTypes<schedulerId>::Sec32Int>
      int32VecMPCValue = std::get<
          std::vector<typename frontend::MPCTypes<schedulerId>::Sec32Int>>(
          deserialization.at("int32VecColumn"));

  std::vector<typename frontend::MPCTypes<schedulerId>::Sec64Int>
      int64VecMPCValue = std::get<
          std::vector<typename frontend::MPCTypes<schedulerId>::Sec64Int>>(
          deserialization.at("int64VecColumn"));

  std::vector<typename frontend::MPCTypes<schedulerId>::SecUnsigned32Int>
      uint32VecMPCValue = std::get<std::vector<
          typename frontend::MPCTypes<schedulerId>::SecUnsigned32Int>>(
          deserialization.at("uint32VecColumn"));

  std::vector<std::vector<int32_t>> int32VecData(
      int32Data.size(), std::vector<int32_t>(int32VecMPCValue.size()));
  std::vector<std::vector<int64_t>> int64VecData(
      int64Data.size(), std::vector<int64_t>(int64VecMPCValue.size()));
  std::vector<std::vector<uint32_t>> uint32VecData(
      uint32Data.size(), std::vector<uint32_t>(uint32VecMPCValue.size()));

  std::vector<int64_t> int64Opened;

  for (int i = 0; i < int32VecMPCValue.size(); i++) {
    int32Opened = int32VecMPCValue[i].openToParty(0).getValue();
    int64Opened = int64VecMPCValue[i].openToParty(0).getValue();
    uint32Opened = uint32VecMPCValue[i].openToParty(0).getValue();

    for (int j = 0; j < int32Opened.size(); j++) {
      int32VecData[j][i] = int32Opened[j];
      int64VecData[j][i] = int64Opened[j];
      uint32VecData[j][i] = uint32Opened[j];
    }
  }

  rst.emplace("int32VecColumn", int32VecData);
  rst.emplace("int64VecColumn", int64VecData);
  rst.emplace("uint32VecColumn", uint32VecData);

  rst.emplace(
      "boolColumn1",
      std::get<typename frontend::MPCTypes<schedulerId>::SecBool>(
          deserialization.at("boolColumn1"))
          .openToParty(0)
          .getValue());
  rst.emplace(
      "boolColumn2",
      std::get<typename frontend::MPCTypes<schedulerId>::SecBool>(
          deserialization.at("boolColumn2"))
          .openToParty(0)
          .getValue());
  rst.emplace(
      "boolColumn3",
      std::get<typename frontend::MPCTypes<schedulerId>::SecBool>(
          deserialization.at("boolColumn3"))
          .openToParty(0)
          .getValue());
  rst.emplace(
      "boolColumn4",
      std::get<typename frontend::MPCTypes<schedulerId>::SecBool>(
          deserialization.at("boolColumn4"))
          .openToParty(0)
          .getValue());

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
  const size_t paddingSize = 7;

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
      createRowDefinition<0>(paddingSize);
  std::unique_ptr<IRowStructureDefinition<1>> serializer1 =
      createRowDefinition<1>(paddingSize);

  EXPECT_EQ(serializer0->getRowSizeBytes(), 17 + 16 * paddingSize);
  EXPECT_EQ(serializer1->getRowSizeBytes(), 17 + 16 * paddingSize);

  std::vector<int32_t> int32Data(0);
  std::vector<int64_t> int64Data(0);
  std::vector<uint32_t> uint32Data(0);
  std::vector<std::vector<int32_t>> int32VecData(0);
  std::vector<std::vector<int64_t>> int64VecData(0);
  std::vector<std::vector<uint32_t>> uint32VecData(0);
  std::vector<bool> boolData1(0);
  std::vector<bool> boolData2(0);
  std::vector<bool> boolData3(0);
  std::vector<bool> boolData4(0);

  for (int i = 0; i < batchSize; i++) {
    int32Data.push_back(int32Dist(e));
    int64Data.push_back(int64Dist(e));
    uint32Data.push_back(uint32Dist(e));

    int32VecData.push_back(std::vector<int32_t>(0));
    int64VecData.push_back(std::vector<int64_t>(0));
    uint32VecData.push_back(std::vector<uint32_t>(0));

    for (int j = 0; j < paddingSize; j++) {
      int32VecData[i].push_back(int32Dist(e));
      int64VecData[i].push_back(int64Dist(e));
      uint32VecData[i].push_back(uint32Dist(e));
    }

    boolData1.push_back(boolDist(e));
    boolData2.push_back(boolDist(e));
    boolData3.push_back(boolDist(e));
    boolData4.push_back(boolDist(e));
  }

  std::unordered_map<std::string, IColumnDefinition<0>::InputColumnDataType>
      inputData{
          {"int32Column", int32Data},
          {"int64Column", int64Data},
          {"uint32Column", uint32Data},
          {"int32VecColumn", int32VecData},
          {"int64VecColumn", int64VecData},
          {"uint32VecColumn", uint32VecData},
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
  auto int32VecRst =
      std::get<std::vector<std::vector<int32_t>>>(rst.at("int32VecColumn"));
  auto int64VecRst =
      std::get<std::vector<std::vector<int64_t>>>(rst.at("int64VecColumn"));
  auto uint32VecRst =
      std::get<std::vector<std::vector<uint32_t>>>(rst.at("uint32VecColumn"));

  testVectorEq(int32Data, int32Rst);
  testVectorEq(int64Data, int64Rst);
  testVectorEq(uint32Data, uint32Rst);
  testVectorEq(boolData1, boolRst1);
  testVectorEq(boolData2, boolRst2);
  testVectorEq(boolData3, boolRst3);
  testVectorEq(boolData4, boolRst4);
  for (int i = 0; i < batchSize; i++) {
    testVectorEq(int32VecData[i], int32VecRst[i]);
    testVectorEq(int64VecData[i], int64VecRst[i]);
    testVectorEq(uint32VecData[i], uint32VecRst[i]);
  }
}

template <int schedulerId>
std::unique_ptr<IRowStructureDefinition<schedulerId>> createBitRowDefinition(
    size_t bitColumns) {
  using ColType = typename IColumnDefinition<schedulerId>::SupportedColumnTypes;
  std::map<std::string, ColType> colDefs;

  for (int i = 0; i < bitColumns; i++) {
    std::string colName = "boolColumn" + std::to_string(i);
    colDefs.emplace(colName, ColType::Bit);
  };

  auto serializer =
      std::make_unique<RowStructureDefinition<schedulerId>>(colDefs, 1);
  return std::move(serializer);
}

template <int schedulerId>
std::unordered_map<std::string, RevealedColumnType>
deserializeAndRevealBitColumns(
    fbpcf::scheduler::ISchedulerFactory<true>& schedulerFactory,
    const std::vector<std::vector<unsigned char>>& serializedSecretShares,
    const std::unique_ptr<IRowStructureDefinition<schedulerId>>& rowDefinition,
    size_t bitColumns) {
  auto udpOutput = deserializeIntoSecString<schedulerId>(
      schedulerFactory, serializedSecretShares);

  auto deserialization =
      rowDefinition.get()->deserializeUDPOutputIntoMPCTypes(udpOutput);
  std::unordered_map<std::string, RevealedColumnType> rst;

  for (int i = 0; i < bitColumns; i++) {
    std::string colName = "boolColumn" + std::to_string(i);
    rst.emplace(
        colName,
        std::get<typename frontend::MPCTypes<schedulerId>::SecBool>(
            deserialization.at(colName))
            .openToParty(0)
            .getValue());
  }

  return rst;
}

TEST(RowSerializationTest, ManyBitsTest) {
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto schedulerFactory0 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          0, *factories[0]);

  auto schedulerFactory1 =
      fbpcf::scheduler::NetworkPlaintextSchedulerFactory<true>(
          1, *factories[1]);

  const size_t batchSize = 100;
  const size_t bitColumns = 95;
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<> boolDist(0, 1);

  std::unique_ptr<IRowStructureDefinition<0>> serializer0 =
      createBitRowDefinition<0>(bitColumns);
  std::unique_ptr<IRowStructureDefinition<1>> serializer1 =
      createBitRowDefinition<1>(bitColumns);

  EXPECT_EQ(
      serializer0->getRowSizeBytes(),
      bitColumns / 8 + (bitColumns % 8 == 0 ? 0 : 1));
  EXPECT_EQ(
      serializer1->getRowSizeBytes(),
      bitColumns / 8 + (bitColumns % 8 == 0 ? 0 : 1));

  std::vector<std::vector<bool>> boolData(
      bitColumns, std::vector<bool>(batchSize));

  std::unordered_map<std::string, IColumnDefinition<0>::InputColumnDataType>
      inputData;
  for (int i = 0; i < bitColumns; i++) {
    for (int j = 0; j < batchSize; j++) {
      boolData[i][j] = boolDist(e);
    }
    inputData.emplace("boolColumn" + std::to_string(i), boolData[i]);
  }

  auto serializedBytes =
      serializer0->serializeDataAsBytesForUDP(inputData, batchSize);

  auto future0 =
      std::async([&schedulerFactory0, &serializedBytes, &serializer0]() {
        return deserializeAndRevealBitColumns<0>(
            schedulerFactory0, serializedBytes, serializer0, bitColumns);
      });

  auto future1 = std::async([&schedulerFactory1, &serializer1]() {
    return deserializeAndRevealBitColumns<1>(
        schedulerFactory1,
        std::vector<std::vector<unsigned char>>(
            batchSize, std::vector<uint8_t>(serializer1->getRowSizeBytes())),
        serializer1,
        bitColumns);
  });

  auto rst = future0.get();
  future1.get();

  for (int i = 0; i < bitColumns; i++) {
    std::string colName = "boolColumn" + std::to_string(i);
    auto values = std::get<std::vector<bool>>(rst.at(colName));
    testVectorEq(values, boolData[i]);
  }
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
