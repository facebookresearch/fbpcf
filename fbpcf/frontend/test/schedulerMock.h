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

class schedulerMock final : public scheduler::IScheduler {
 public:
  schedulerMock() {
    ON_CALL(*this, privateBooleanInput(_, _))
        .WillByDefault(Return(WireId<IScheduler::Boolean>(1)));

    ON_CALL(*this, publicBooleanInput(_))
        .WillByDefault(Return(WireId<IScheduler::Boolean>(1)));

    ON_CALL(*this, privateBooleanInputBatch(_, _))
        .WillByDefault(
            Invoke([](const std::vector<bool>& /*input*/, int /*party*/) {
              return WireId<IScheduler::Boolean>(1);
            }));

    ON_CALL(*this, publicBooleanInputBatch(_))
        .WillByDefault(Invoke([](const std::vector<bool>& /*input*/) {
          return WireId<IScheduler::Boolean>(1);
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

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }

  std::pair<uint64_t, uint64_t> getWireStatistics() const override {
    return {0, 0};
  }
};

} // namespace fbpcf::frontend
