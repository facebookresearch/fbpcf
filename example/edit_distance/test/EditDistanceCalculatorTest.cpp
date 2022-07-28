/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../EditDistanceCalculator.h" // @manual
#include <fbpcf/scheduler/SchedulerHelper.h>
#include <folly/json.h>
#include <gtest/gtest.h>
#include "../EditDistanceInputReader.h" // @manual
#include "../MPCTypes.h" // @manual
#include "./Constants.h" // @manual
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/test/TestHelper.h"
#include "tools/cxx/Resources.h"

namespace fbpcf::edit_distance {

template <int schedulerId>
EditDistanceCalculator<schedulerId> createCalculatorWithScheduler(
    int myRole,
    EditDistanceInputReader&& inputData,
    std::reference_wrapper<
        fbpcf::engine::communication::IPartyCommunicationAgentFactory> factory,
    fbpcf::SchedulerCreator schedulerCreator) {
  auto scheduler = schedulerCreator(myRole, factory);
  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));
  InputProcessor<schedulerId> inputProcessor(myRole, std::move(inputData));
  return EditDistanceCalculator<schedulerId>(myRole, std::move(inputProcessor));
}

template <int schedulerId>
std::pair<std::vector<int64_t>, std::vector<std::string>> revealOutputs(
    int myRole,
    EditDistanceCalculator<schedulerId>& calculator) {
  if (myRole == 0) {
    auto distances =
        calculator.getEditDistances().openToParty(PLAYER0).getValue();
    auto receiverMessages =
        calculator.getReceiverMessages().openToParty(PLAYER0).getValue();

    calculator.getEditDistances().openToParty(PLAYER1).getValue();
    calculator.getReceiverMessages().openToParty(PLAYER1).getValue();
    return std::pair(distances, receiverMessages);
  } else {
    calculator.getEditDistances().openToParty(PLAYER0).getValue();
    calculator.getReceiverMessages().openToParty(PLAYER0).getValue();

    auto distances =
        calculator.getEditDistances().openToParty(PLAYER1).getValue();
    auto receiverMessages =
        calculator.getReceiverMessages().openToParty(PLAYER1).getValue();
    return std::pair(distances, receiverMessages);
  }
}

TEST(EditDistanceCalculatorTest, testEditDistanceCalculator) {
  boost::filesystem::path dataFilepath1 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));

  boost::filesystem::path dataFilepath2 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));

  auto player0InputData =
      EditDistanceInputReader(dataFilepath1.native(), paramsFilePath.native());

  auto player1InputData =
      EditDistanceInputReader(dataFilepath2.native(), paramsFilePath.native());

  auto schedulerCreator = fbpcf::scheduler::createLazySchedulerWithRealEngine;
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto future0 = std::async(
      createCalculatorWithScheduler<0>,
      PLAYER0,
      std::move(player0InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[0]),
      schedulerCreator);

  auto future1 = std::async(
      createCalculatorWithScheduler<1>,
      PLAYER1,
      std::move(player1InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[1]),
      schedulerCreator);

  EditDistanceCalculator<0> player0Calculator = future0.get();
  EditDistanceCalculator<1> player1Calculator = future1.get();

  auto future2 = std::async(
      revealOutputs<0>,
      PLAYER0,
      std::reference_wrapper<EditDistanceCalculator<0>>(player0Calculator));

  auto future3 = std::async(
      revealOutputs<1>,
      PLAYER1,
      std::reference_wrapper<EditDistanceCalculator<1>>(player1Calculator));

  auto player0Results = future2.get();
  auto player1Results = future3.get();

  testVectorEq(std::get<0>(player0Results), kExpectedDistances);
  testVectorEq(std::get<1>(player0Results), kExpectedReceiverMessages);

  testVectorEq(std::get<0>(player1Results), kExpectedDistances);
  testVectorEq(std::get<1>(player1Results), kExpectedReceiverMessages);
}

TEST(EditDistanceCalculatorTest, testToJson) {
  boost::filesystem::path dataFilepath1 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));

  boost::filesystem::path dataFilepath2 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));

  auto player0InputData =
      EditDistanceInputReader(dataFilepath1.native(), paramsFilePath.native());

  auto player1InputData =
      EditDistanceInputReader(dataFilepath2.native(), paramsFilePath.native());

  auto schedulerCreator = fbpcf::scheduler::createLazySchedulerWithRealEngine;
  auto factories = fbpcf::engine::communication::getInMemoryAgentFactory(2);

  auto future0 = std::async(
      createCalculatorWithScheduler<0>,
      PLAYER0,
      std::move(player0InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[0]),
      schedulerCreator);

  auto future1 = std::async(
      createCalculatorWithScheduler<1>,
      PLAYER1,
      std::move(player1InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[1]),
      schedulerCreator);

  EditDistanceCalculator<0> player0Calculator = future0.get();
  EditDistanceCalculator<1> player1Calculator = future1.get();

  auto player0SharesJson = player0Calculator.toJson();
  auto player1SharesJson = player1Calculator.toJson();

  folly::dynamic player0Shares = folly::parseJson(player0SharesJson);
  folly::dynamic player1Shares = folly::parseJson(player1SharesJson);

  size_t numRows = player0Shares["editDistanceShares"].size();

  std::vector<int64_t> editDistancesRecovered(numRows);
  std::vector<std::string> receiverMessagesRecovered(numRows);

  for (int i = 0; i < numRows; i++) {
    editDistancesRecovered[i] = player0Shares["editDistanceShares"][i].asInt() ^
        player1Shares["editDistanceShares"][i].asInt();

    std::string strShare0 =
        player0Shares["receiverMessageShares"][i].asString();
    std::string strShare1 =
        player1Shares["receiverMessageShares"][i].asString();
    std::string recoveredMessage;
    for (int j = 0; j < maxStringLength; j++) {
      char c = (strShare0.size() > j ? strShare0[j] : 0) ^
          (strShare1.size() > j ? strShare1[j] : 0);
      if (c == 0) {
        break;
      } else {
        recoveredMessage.push_back(c);
      }
    }
    receiverMessagesRecovered[i] = recoveredMessage;
  }
  testVectorEq(editDistancesRecovered, kExpectedDistances);
  testVectorEq(receiverMessagesRecovered, kExpectedReceiverMessages);
}
} // namespace fbpcf::edit_distance
