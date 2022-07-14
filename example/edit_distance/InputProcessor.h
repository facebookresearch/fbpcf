/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "./EditDistanceInputReader.h" // @manual
#include "./MPCTypes.h" // @manual
#include "folly/logging/xlog.h"

#pragma once

namespace fbpcf::edit_distance {

template <int schedulerId>
class InputProcessor {
 public:
  explicit InputProcessor(int myRole, EditDistanceInputReader&& inputData)
      : myRole_{myRole}, inputData_{inputData} {
    validateNumRows();
    validateParameters();
    privatelyShareWordsStep();
    privatelyShareGuessesStep();
    privatelyShareSenderMessagesStep();
  }

  InputProcessor() {}

  const EditDistanceInputReader& getInputData() const {
    return inputData_;
  }

  const SecString<schedulerId>& getWords() const {
    return words_;
  }

  const SecString<schedulerId>& getGuesses() const {
    return guesses_;
  }

  const SecString<schedulerId>& getSenderMessages() const {
    return senderMessages_;
  }

  const Pub32Int<schedulerId>& getThreshold() const {
    return threshold_;
  }

  const Pub32Int<schedulerId>& getDeleteCost() const {
    return deleteCost_;
  }

  const Pub32Int<schedulerId>& getInsertCost() const {
    return insertCost_;
  }

 private:
  void validateNumRows();

  void validateParameters();

  void privatelyShareWordsStep();

  void privatelyShareGuessesStep();

  void privatelyShareSenderMessagesStep();

  int myRole_;
  EditDistanceInputReader inputData_;

  SecString<schedulerId> words_;
  SecString<schedulerId> guesses_;
  SecString<schedulerId> senderMessages_;

  Pub32Int<schedulerId> threshold_;
  Pub32Int<schedulerId> deleteCost_;
  Pub32Int<schedulerId> insertCost_;
};

} // namespace fbpcf::edit_distance

#include "./InputProcessor_impl.h" // @manual
