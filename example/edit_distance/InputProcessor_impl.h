/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "./InputProcessor.h" // @manual
#include "./MPCTypes.h" // @manual
#include "./Util.h" // @manual

#pragma once

namespace fbpcf::edit_distance {

template <int schedulerId>
void InputProcessor<schedulerId>::validateNumRows() {
  XLOG(INFO) << "Sharing number of rows";
  int player0NumRows = shareIntFrom<schedulerId, int32Length, PLAYER0, PLAYER1>(
      myRole_, inputData_.getNumRows());
  int player1NumRows = shareIntFrom<schedulerId, int32Length, PLAYER1, PLAYER0>(
      myRole_, inputData_.getNumRows());

  if (player0NumRows != player1NumRows) {
    throw std::runtime_error(fmt::format(
        "Player 0 has {} rows. Player 1 has {} rows. Should be equal.",
        player0NumRows,
        player1NumRows));
  }
}

template <int schedulerId>
void InputProcessor<schedulerId>::validateParameters() {
  XLOG(INFO) << "Sharing message threshold";
  int player0Threshold =
      shareIntFrom<schedulerId, int32Length, PLAYER0, PLAYER1>(
          myRole_, inputData_.getThreshold());
  int player1Threshold =
      shareIntFrom<schedulerId, int32Length, PLAYER1, PLAYER0>(
          myRole_, inputData_.getThreshold());

  if (player0Threshold != player1Threshold) {
    throw std::runtime_error(fmt::format(
        "Player 0 has threshold = {}. Player 1 has threshold = {}. Should be equal.",
        player0Threshold,
        player1Threshold));
  }

  threshold_ = createPublicBatchConstant<Pub32Int<schedulerId>, int64_t>(
      inputData_.getThreshold(), inputData_.getNumRows());

  XLOG(INFO) << "Sharing delete cost";

  int player0DeleteCost =
      shareIntFrom<schedulerId, int32Length, PLAYER0, PLAYER1>(
          myRole_, inputData_.getDeleteCost());

  int player1DeleteCost =
      shareIntFrom<schedulerId, int32Length, PLAYER1, PLAYER0>(
          myRole_, inputData_.getDeleteCost());

  if (player0DeleteCost != player1DeleteCost) {
    throw std::runtime_error(fmt::format(
        "Player 0 has delete_cost = {}. Player 1 has delete_cost = {}. Should be equal.",
        player0DeleteCost,
        player1DeleteCost));
  }

  deleteCost_ = createPublicBatchConstant<Pub32Int<schedulerId>, int64_t>(
      inputData_.getDeleteCost(), inputData_.getNumRows());

  XLOG(INFO) << "Sharing insertion cost";

  int player0InsertCost =
      shareIntFrom<schedulerId, int32Length, PLAYER0, PLAYER1>(
          myRole_, inputData_.getInsertCost());

  int player1InsertCost =
      shareIntFrom<schedulerId, int32Length, PLAYER1, PLAYER0>(
          myRole_, inputData_.getInsertCost());

  if (player0InsertCost != player1InsertCost) {
    throw std::runtime_error(fmt::format(
        "Player 0 has insert_cost = {}. Player 1 has insert_cost = {}. Should be equal.",
        player0InsertCost,
        player1InsertCost));
  }

  insertCost_ = createPublicBatchConstant<Pub32Int<schedulerId>, int64_t>(
      inputData_.getInsertCost(), inputData_.getNumRows());
}

template <int schedulerId>
void InputProcessor<schedulerId>::privatelyShareWordsStep() {
  XLOG(INFO) << "Share words step";
  words_ = SecString<schedulerId>(inputData_.getWords(), PLAYER0);
}

template <int schedulerId>
void InputProcessor<schedulerId>::privatelyShareGuessesStep() {
  XLOG(INFO) << "Share guesses step";
  guesses_ = SecString<schedulerId>(inputData_.getGuesses(), PLAYER1);
}

template <int schedulerId>
void InputProcessor<schedulerId>::privatelyShareSenderMessagesStep() {
  XLOG(INFO) << "Share sender messages step";
  senderMessages_ =
      SecString<schedulerId>(inputData_.getSenderMessages(), PLAYER0);
}

} // namespace fbpcf::edit_distance
