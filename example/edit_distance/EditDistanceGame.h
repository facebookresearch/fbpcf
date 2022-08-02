/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/engine/communication/IPartyCommunicationAgentFactory.h>
#include <memory>
#include "./EditDistanceCalculator.h" // @manual
#include "./EditDistanceInputReader.h" // @manual
#include "./InputProcessor.h" // @manual
#include "fbpcf/frontend/mpcGame.h"

namespace fbpcf::edit_distance {

template <int schedulerId>
class EditDistanceGame : public fbpcf::frontend::MpcGame<schedulerId> {
 public:
  explicit EditDistanceGame(
      int party,
      std::unique_ptr<fbpcf::scheduler::IScheduler> scheduler)
      : fbpcf::frontend::MpcGame<schedulerId>(std::move(scheduler)),
        party_{party} {}

  /**
   * Runs the EditDistance logic given a set of inputs
   * using the scheduler provided.
   *
   * @returns A JSON string in the format
   * {
   *   editDistanceShares: [int],
   *   receiverMessageShares: [string]
   * }
   * which are the parties shares of the game results.
   **/
  std::string play(EditDistanceInputReader&& inputReader) {
    auto inputProcessor =
        InputProcessor<schedulerId>(party_, std::move(inputReader));
    auto calculator =
        EditDistanceCalculator<schedulerId>(party_, std::move(inputProcessor));
    return calculator.toJson();
  }

 private:
  const int party_;
};

} // namespace fbpcf::edit_distance
