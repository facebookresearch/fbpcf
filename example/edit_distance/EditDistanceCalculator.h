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
class EditDistanceCalculator {
 public:
  explicit EditDistanceCalculator(
      int myRole,
      InputProcessor<schedulerId>&& inputProcessor)
      : myRole_{myRole}, inputProcessor_(inputProcessor) {
    calculateEditDistances();
    calculateMessages();
  }

  InputProcessor<schedulerId>& getInputProcessor() {
    return inputProcessor_;
  }

  SecString<schedulerId>& getReceiverMessages() {
    return receiverMessages_;
  }

  Sec32Int<schedulerId> getEditDistances() {
    return editDistances_;
  }

 private:
  void calculateEditDistances();
  void calculateMessages();

  int myRole_;
  InputProcessor<schedulerId> inputProcessor_;

  SecString<schedulerId> receiverMessages_;
  Sec32Int<schedulerId> editDistances_;
};

} // namespace fbpcf::edit_distance

#include "./EditDistanceCalculator_impl.h" // @manual
