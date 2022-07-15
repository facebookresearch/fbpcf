/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../InputProcessor.h" // @manual
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
InputProcessor<schedulerId> createInputProcessorWithScheduler(
    int myRole,
    EditDistanceInputReader&& inputData,
    std::reference_wrapper<
        fbpcf::engine::communication::IPartyCommunicationAgentFactory> factory,
    fbpcf::SchedulerCreator schedulerCreator) {
  auto scheduler = schedulerCreator(myRole, factory);
  fbpcf::scheduler::SchedulerKeeper<schedulerId>::setScheduler(
      std::move(scheduler));
  return InputProcessor<schedulerId>(myRole, std::move(inputData));
}

template <int schedulerId>
std::tuple<
    std::vector<std::string>,
    std::vector<std::string>,
    std::vector<std::string>>
revealOutputs(int myRole, InputProcessor<schedulerId>& inputProcessor) {
  if (myRole == 0) {
    auto words = inputProcessor.getWords().openToParty(PLAYER0).getValue();
    auto guesses = inputProcessor.getGuesses().openToParty(PLAYER0).getValue();
    auto senderMessages =
        inputProcessor.getSenderMessages().openToParty(PLAYER0).getValue();

    inputProcessor.getWords().openToParty(PLAYER1);
    inputProcessor.getGuesses().openToParty(PLAYER1);
    inputProcessor.getSenderMessages().openToParty(PLAYER1);

    return std::tuple(words, guesses, senderMessages);
  } else {
    inputProcessor.getWords().openToParty(PLAYER0);
    inputProcessor.getGuesses().openToParty(PLAYER0);
    inputProcessor.getSenderMessages().openToParty(PLAYER0);

    auto words = inputProcessor.getWords().openToParty(PLAYER1).getValue();
    auto guesses = inputProcessor.getGuesses().openToParty(PLAYER1).getValue();
    auto senderMessages =
        inputProcessor.getSenderMessages().openToParty(PLAYER1).getValue();

    return std::tuple(words, guesses, senderMessages);
  }
}

TEST(InputProcessorTest, testInputProcessor) {
  using namespace fbcpf::edit_distance;

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
      createInputProcessorWithScheduler<0>,
      PLAYER0,
      std::move(player0InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[0]),
      schedulerCreator);

  auto future1 = std::async(
      createInputProcessorWithScheduler<1>,
      PLAYER1,
      std::move(player1InputData),
      std::reference_wrapper<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>(
          *factories[1]),
      schedulerCreator);

  InputProcessor<0> player0InputProcessor = future0.get();
  InputProcessor<1> player1InputProcessor = future1.get();

  testVectorEq(
      player0InputProcessor.getThreshold().getValue(), kExpectedThresholdBatch);
  testVectorEq(
      player1InputProcessor.getThreshold().getValue(), kExpectedThresholdBatch);

  testVectorEq(
      player0InputProcessor.getDeleteCost().getValue(), kExpectedDeleteBatch);
  testVectorEq(
      player1InputProcessor.getDeleteCost().getValue(), kExpectedDeleteBatch);

  testVectorEq(
      player0InputProcessor.getInsertCost().getValue(), kExpectedInsertBatch);
  testVectorEq(
      player1InputProcessor.getInsertCost().getValue(), kExpectedInsertBatch);

  auto future2 = std::async(
      revealOutputs<0>,
      PLAYER0,
      std::reference_wrapper<InputProcessor<0>>(player0InputProcessor));

  auto future3 = std::async(
      revealOutputs<1>,
      PLAYER1,
      std::reference_wrapper<InputProcessor<1>>(player1InputProcessor));

  auto player0Results = future2.get();
  auto player1Results = future3.get();

  testVectorEq(std::get<0>(player0Results), kExpectedWords);
  testVectorEq(std::get<1>(player0Results), kExpectedGuesses);
  testVectorEq(std::get<2>(player0Results), kExpectedSenderMessages);

  testVectorEq(std::get<0>(player1Results), kExpectedWords);
  testVectorEq(std::get<1>(player1Results), kExpectedGuesses);
  testVectorEq(std::get<2>(player1Results), kExpectedSenderMessages);
}
} // namespace fbpcf::edit_distance
