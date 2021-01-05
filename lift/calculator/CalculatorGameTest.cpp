/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "../../pcf/mpc/EmpGame.h"
#include "../../pcf/mpc/EmpTestUtil.h"
#include "../../pcf/mpc/QueueIO.h"
#include "../common/Csv.h"
#include "../common/GroupedLiftMetrics.h"
#include "CalculatorGame.h"
#include "CalculatorGameConfig.h"
#include "InputData.h"
#include "OutputMetrics.h"
#include "folly/Random.h"
#include "test/common/GenFakeData.h"
#include "test/common/LiftCalculator.h"

namespace private_lift {
class CalculatorGameTest : public ::testing::Test {
 public:
  CalculatorGameConfig getInputData(
      const std::filesystem::path& inputPath,
      bool isConversionLift) {
    int32_t numConversionsPerUser = isConversionLift ? 4 : 1;
    int64_t epoch = 1546300800;

    auto liftGranularityType = isConversionLift
        ? InputData::LiftGranularityType::Conversion
        : InputData::LiftGranularityType::Converter;

    InputData inputData{inputPath,
                        InputData::LiftMPCType::Standard,
                        liftGranularityType,
                        epoch,
                        numConversionsPerUser};
    CalculatorGameConfig config = {
        inputData, isConversionLift, numConversionsPerUser};
    return config;
  }

 protected:
  void SetUp() override {}
};

TEST_F(CalculatorGameTest, TestRandomInputConversionLift) {
  std::string aliceInputFilename =
      "publisher_" + std::to_string(folly::Random::secureRand64()) + ".csv";
  std::string bobInputFilename =
      "partner_" + std::to_string(folly::Random::secureRand64()) + ".csv";

  // generate test input files with random data
  GenFakeData testDataGenerator;
  testDataGenerator.genFakePublisherInputFile(
      aliceInputFilename,
      15 /* numRows*/,
      0.5 /* opportunityRate */,
      0.5 /* testRate */,
      0.5 /* purchaseRate */,
      0.0 /* incrementalityRate */,
      1546300800 /* epoch */);
  testDataGenerator.genFakePartnerInputFile(
      bobInputFilename,
      15,
      0.5,
      0.5,
      0.5,
      0.0,
      1546300800,
      4 /* numConversionsPerRow */);
  CalculatorGameConfig configRandomConversionAlice =
      CalculatorGameTest::getInputData(aliceInputFilename, true);
  CalculatorGameConfig configRandomConversionBob =
      CalculatorGameTest::getInputData(bobInputFilename, true);

  // compute results with CalculatorGame
  auto res = pcf::mpc::
      test<CalculatorGame<pcf::QueueIO>, CalculatorGameConfig, std::string>(
          configRandomConversionAlice, configRandomConversionBob);
  GroupedLiftMetrics resFirst = GroupedLiftMetrics::fromJson(res.first);
  GroupedLiftMetrics resSecond = GroupedLiftMetrics::fromJson(res.second);

  // calculate expected results with simple lift calculator
  LiftCalculator liftCalculator;
  std::ifstream inFileAlice{aliceInputFilename};
  std::ifstream inFileBob{bobInputFilename};
  int32_t tsOffset = 10;
  std::string linePublisher;
  std::string linePartner;
  getline(inFileAlice, linePublisher);
  getline(inFileBob, linePartner);
  auto headerPublisher = csv::splitByComma(linePublisher, false);
  auto headerPartner = csv::splitByComma(linePartner, false);
  std::unordered_map<std::string, int> colNameToIndex =
      liftCalculator.mapColToIndex(headerPublisher, headerPartner);
  OutputMetricsData computedResult =
      liftCalculator.compute(inFileAlice, inFileBob, colNameToIndex, tsOffset);
  GroupedLiftMetrics expectedRes;
  expectedRes.metrics = computedResult.toLiftMetrics();

  // remove test input files
  std::filesystem::remove(aliceInputFilename);
  std::filesystem::remove(bobInputFilename);

  // assert expected results and CalculatorGame calculated results
  EXPECT_EQ(expectedRes, resFirst);
  EXPECT_EQ(expectedRes, resSecond);
}
} // namespace private_lift
