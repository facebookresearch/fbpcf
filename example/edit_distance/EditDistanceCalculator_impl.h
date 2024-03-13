/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/json.h>
#include <stdexcept>
#include "./EditDistanceCalculator.h" // @manual
#include "folly/json/dynamic.h"
#include "folly/logging/xlog.h"

namespace fbpcf::edit_distance {

template <int schedulerId>
void EditDistanceCalculator<schedulerId>::calculateEditDistances() {
  XLOG(INFO, "Start edit distance calculation");
  const SecString<schedulerId>& words = inputProcessor_.getWords();
  const SecString<schedulerId>& guesses = inputProcessor_.getGuesses();

  SecUChar<schedulerId> wordsSize = words.template privateSize<charLength>();
  SecUChar<schedulerId> guessSize = guesses.template privateSize<charLength>();

  size_t batchSize = inputProcessor_.getInputData().getNumRows();
  Pub32Int<schedulerId> zero =
      createPublicBatchConstant<Pub32Int<schedulerId>, int64_t>(0, batchSize);
  const Pub32Int<schedulerId>& insertCost = inputProcessor_.getInsertCost();
  const Pub32Int<schedulerId>& deleteCost = inputProcessor_.getDeleteCost();

  std::vector<SecBool<schedulerId>> in_bounds_word(maxStringLength);
  std::vector<SecBool<schedulerId>> in_bounds_guess(maxStringLength);
  for (size_t i = 0; i < maxStringLength; i++) {
    PubUChar<schedulerId> index =
        createPublicBatchConstant<PubUChar<schedulerId>, size_t>(i, batchSize);
    in_bounds_word[i] = index < wordsSize;
    in_bounds_guess[i] = index < guessSize;
  }

  std::vector<std::vector<Sec32Int<schedulerId>>> distances(
      maxStringLength + 1);

  distances[0] = std::vector<Sec32Int<schedulerId>>(maxStringLength + 1);

  // Not technically a secret value but makes the types work more easily
  distances[0][0] = createSecretBatchConstant<Sec32Int<schedulerId>, int64_t>(
      0, batchSize, PLAYER0);

  for (size_t i = 1; i <= maxStringLength; i++) {
    distances[i] = std::vector<Sec32Int<schedulerId>>(maxStringLength + 1);
    distances[0][i] =
        distances[0][i - 1] + zero.mux(in_bounds_guess[i - 1], deleteCost);
    distances[i][0] =
        distances[i - 1][0] + zero.mux(in_bounds_word[i - 1], insertCost);
  }

  for (size_t i = 1; i <= maxStringLength; i++) {
    for (size_t j = 1; j <= maxStringLength; j++) {
      // cast signed char to signed 32int
      SecChar<schedulerId> wordLetter = words[i - 1];
      SecChar<schedulerId> guessLetter = guesses[j - 1];
      Sec32Int<schedulerId> asciiDist =
          (wordLetter - guessLetter)
              .mux(wordLetter < guessLetter, guessLetter - wordLetter)
              .template cast<int32Length>();
      auto replaceSol = distances[i - 1][j - 1] + asciiDist;
      auto insertSol = distances[i - 1][j] + insertCost;
      auto deleteSol = distances[i][j - 1] + deleteCost;
      auto minSol = fbpcf::frontend::min(
          fbpcf::frontend::min(replaceSol, insertSol), deleteSol);
      /*
      Case 1: In Bounds Both Words
          - Return minSolution
      Case 2: In Bounds of Guess / Not in bounds of word
          - Copy solution of wordIndex - 1
      Case 3: In bounds of Word / Not in bounds of guess
          - Copy solution of guessIndex - 1
      Case 4: Not in bounds of either
          - Copy solution of wordIndex - 1, guessIndex - 1
      */
      distances[i][j] =
          distances[i - 1][j - 1]
              .mux(in_bounds_word[i - 1], distances[i][j - 1])
              .mux(
                  in_bounds_guess[j - 1],
                  distances[i - 1][j].mux(
                      in_bounds_word[i - 1] & in_bounds_guess[j - 1], minSol));
    }
  }

  editDistances_ = distances[maxStringLength][maxStringLength];

  // force computation of result in this class
  editDistances_.extractIntShare();
  XLOG(INFO, "Finished Edit Distance Calculation");
}

template <int schedulerId>
void EditDistanceCalculator<schedulerId>::calculateMessages() {
  XLOG(INFO, "Start message calculation");
  const Pub32Int<schedulerId>& threshold = inputProcessor_.getThreshold();
  SecUChar<schedulerId> messageLength =
      inputProcessor_.getSenderMessages().template privateSize<charLength>();

  size_t batchSize = inputProcessor_.getInputData().getNumRows();
  PubString<schedulerId> emptyMessage =
      createPublicBatchConstant<PubString<schedulerId>, std::string>(
          "", batchSize);
  PubChar<schedulerId> nullChar =
      createPublicBatchConstant<PubChar<schedulerId>, int64_t>(0, batchSize);
  PubUChar<schedulerId> one =
      createPublicBatchConstant<PubUChar<schedulerId>, uint64_t>(1, batchSize);
  const SecString<schedulerId>& senderMessage =
      inputProcessor_.getSenderMessages();
  SecString<schedulerId> halfMessage;
  for (size_t i = 0; i < maxStringLength; i++) {
    PubUChar<schedulerId> index =
        createPublicBatchConstant<PubUChar<schedulerId>, uint64_t>(
            i, batchSize);
    halfMessage[i] =
        nullChar.mux((index + index + one) < messageLength, senderMessage[i]);
  }

  receiverMessages_ =
      emptyMessage.mux(editDistances_ <= threshold + threshold, halfMessage)
          .mux(editDistances_ < threshold, senderMessage);
  // force calculation of results in this class
  receiverMessages_.extractAsciiStringShare();
  XLOG(INFO, "Finish message calculation");
}

template <int schedulerId>
std::string EditDistanceCalculator<schedulerId>::toJson() const {
  folly::dynamic output = folly::dynamic::object;
  std::vector<int64_t> editDistanceShares =
      editDistances_.extractIntShare().getValue();
  std::vector<std::string> receiverMessageShares =
      receiverMessages_.extractAsciiStringShare().getValue();
  output["editDistanceShares"] =
      folly::dynamic(editDistanceShares.begin(), editDistanceShares.end());
  output["receiverMessageShares"] = folly::dynamic::array(
      receiverMessageShares.begin(), receiverMessageShares.end());
  return folly::toJson(output);
}
} // namespace fbpcf::edit_distance
