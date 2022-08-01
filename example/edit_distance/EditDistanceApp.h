/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/scheduler/IScheduler.h>
#include <memory>
#include <string>
#include <vector>
#include "./EditDistanceGame.h" // @manual
#include "./EditDistanceInputReader.h" // @manual
#include "./InputProcessor.h" // @manual
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"

namespace fbpcf::edit_distance {

template <int schedulerId>
class EditDistanceApp {
 public:
  explicit EditDistanceApp(
      int myRole,
      std::unique_ptr<
          fbpcf::engine::communication::IPartyCommunicationAgentFactory>
          communicationAgentFactory,
      const std::string& dataFilePath,
      const std::string& paramsFilePath,
      const std::string& outFilePath)
      : myRole_{myRole},
        communicationAgentFactory_{std::move(communicationAgentFactory)},
        dataFilePath_{dataFilePath},
        paramsFilePath_{paramsFilePath},
        outFilePath_{outFilePath} {}

  /**
   * Creates a secure scheduler assigned to schedulerId in the template
   * parameter which is then used to calculate the edit distance game given the
   * input data and game parameters. Writes the results to the provided output
   * path.
   **/
  void run();

 private:
  int myRole_;
  std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgentFactory>
      communicationAgentFactory_;
  std::string dataFilePath_;
  std::string paramsFilePath_;
  std::string outFilePath_;
};

} // namespace fbpcf::edit_distance

#include "./EditDistanceApp_impl.h" // @manual
