/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/scheduler/IScheduler.h"

namespace fbpcf::frontend {

using namespace ::testing;

MATCHER_P(WireIdEq, expectedId, "Check if it is the expected WireId") {
  return expectedId == arg.getId();
}

MATCHER_P(
    WireIdVectorEq,
    expectedIdVector,
    "Check if it is the expected WireId vector") {
  if (arg.size() != expectedIdVector.size()) {
    return false;
  }
  for (size_t i = 0; i < arg.size(); i++) {
    if (arg.at(i).getId() != expectedIdVector.at(i)) {
      return false;
    }
  }
  return true;
}

MATCHER_P(VectorPtrEq, expectedVector, "Check if it is the expected Vector") {
  if (arg->size() != expectedVector.size()) {
    return false;
  }
  for (size_t i = 0; i < arg->size(); i++) {
    if (arg->at(i) != expectedVector.at(i)) {
      return false;
    }
  }
  return true;
}

class schedulerMock final : public scheduler::IScheduler {
 public:
  schedulerMock() {
    ON_CALL(*this, privateBooleanInput(_, _))
        .WillByDefault(Invoke([this](bool /*input*/, int /*party*/) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, publicBooleanInput(_))
        .WillByDefault(Invoke([this](bool /*input*/) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateBooleanInputBatch(_, _))
        .WillByDefault(
            Invoke([this](const std::vector<bool>& /*input*/, int /*party*/) {
              return WireId<IScheduler::Boolean>(wireId++);
            }));

    ON_CALL(*this, publicBooleanInputBatch(_))
        .WillByDefault(Invoke([this](const std::vector<bool>& /*input*/) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateAndPrivate(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateXorPrivate(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateAndPublic(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateXorPublic(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, publicAndPublic(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, publicXorPublic(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateAndPrivateBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateXorPrivateBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateAndPublicBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, privateXorPublicBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, publicAndPublicBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, publicXorPublicBatch(_, _))
        .WillByDefault(Invoke([this](auto, auto) {
          return WireId<IScheduler::Boolean>(wireId++);
        }));

    ON_CALL(*this, batchingUp(_)).WillByDefault(Invoke([this](auto) {
      return WireId<IScheduler::Boolean>(wireId++);
    }));

    ON_CALL(*this, unbatching(_, _))
        .WillByDefault(Invoke([this](auto, auto strategy) {
          std::vector<WireId<IScheduler::Boolean>> rst(strategy->size());
          for (auto& item : rst) {
            item = WireId<IScheduler::Boolean>(wireId++);
          }
          return rst;
        }));
  }

  //======== Below are input processing APIs: ========
  MOCK_METHOD2(privateBooleanInput, WireId<IScheduler::Boolean>(bool, int));

  MOCK_METHOD2(
      privateBooleanInputBatch,
      WireId<IScheduler::Boolean>(const std::vector<bool>&, int));

  MOCK_METHOD1(publicBooleanInput, WireId<IScheduler::Boolean>(bool));

  MOCK_METHOD1(
      publicBooleanInputBatch,
      WireId<IScheduler::Boolean>(const std::vector<bool>&));

  MOCK_METHOD1(recoverBooleanWire, WireId<IScheduler::Boolean>(bool));

  MOCK_METHOD1(
      recoverBooleanWireBatch,
      WireId<IScheduler::Boolean>(const std::vector<bool>&));

  //======== Below are output processing APIs: ========

  MOCK_METHOD2(
      openBooleanValueToParty,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>, int));

  MOCK_METHOD2(
      openBooleanValueToPartyBatch,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>, int));

  MOCK_METHOD1(extractBooleanSecretShare, bool(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(
      extractBooleanSecretShareBatch,
      std::vector<bool>(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(getBooleanValue, bool(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(
      getBooleanValueBatch,
      std::vector<bool>(WireId<IScheduler::Boolean>));

  //======== Below are computation APIs: ========

  // ------ AND gates ------

  MOCK_METHOD2(
      privateAndPrivate,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateAndPrivateBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateAndPublic,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateAndPublicBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      publicAndPublic,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      publicAndPublicBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  // ------ Composite AND gates ------

  MOCK_METHOD2(
      privateAndPrivateComposite,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      privateAndPrivateCompositeBatch,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      privateAndPublicComposite,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      privateAndPublicCompositeBatch,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      publicAndPublicComposite,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      publicAndPublicCompositeBatch,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::vector<WireId<Boolean>>));

  // ------ XOR gates ------

  MOCK_METHOD2(
      privateXorPrivate,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateXorPrivateBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateXorPublic,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      privateXorPublicBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      publicXorPublic,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  MOCK_METHOD2(
      publicXorPublicBatch,
      WireId<IScheduler::Boolean>(
          WireId<IScheduler::Boolean>,
          WireId<IScheduler::Boolean>));

  // ------ Not gates ------

  MOCK_METHOD1(
      notPrivate,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(
      notPrivateBatch,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(
      notPublic,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(
      notPublicBatch,
      WireId<IScheduler::Boolean>(WireId<IScheduler::Boolean>));

  //======== Below are wire management APIs: ========

  MOCK_METHOD1(increaseReferenceCount, void(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(increaseReferenceCountBatch, void(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(decreaseReferenceCount, void(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(decreaseReferenceCountBatch, void(WireId<IScheduler::Boolean>));

  MOCK_METHOD1(batchingUp, WireId<Boolean>(std::vector<WireId<Boolean>>));

  MOCK_METHOD2(
      unbatching,
      std::vector<WireId<Boolean>>(
          WireId<Boolean>,
          std::shared_ptr<std::vector<uint32_t>>));

  MOCK_METHOD0(deleteEngine, void());

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }

  std::pair<uint64_t, uint64_t> getWireStatistics() const override {
    return {0, 0};
  }

  size_t getBatchSize(
      IScheduler::WireId<IScheduler::Boolean> /*id*/) const override {
    return 0;
  }

  size_t getBatchSize(
      IScheduler::WireId<IScheduler::Arithmetic> /*id*/) const override {
    return 0;
  }

 private:
  int wireId = 1;
};

} // namespace fbpcf::frontend
