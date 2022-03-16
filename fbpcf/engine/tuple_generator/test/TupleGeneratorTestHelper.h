/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <functional>
#include <iterator>
#include <map>
#include <memory>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/tuple_generator/DummyProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/DummyTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/TupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/TwoPartyTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"

#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"

namespace fbpcf::engine::tuple_generator {
const uint64_t kTestExtendedSize = 2048;
const uint64_t kTestBaseSize = 1024;
const uint64_t kTestWeight = 16;
const uint64_t kTestBufferSize = 1024;

inline std::unique_ptr<ITupleGeneratorFactory> createDummyTupleGeneratorFactory(
    int /*numberOfParty*/,
    int /*myId*/,
    communication::IPartyCommunicationAgentFactory& /*agentFactory*/) {
  return std::make_unique<insecure::DummyTupleGeneratorFactory>();
}

inline std::unique_ptr<ITupleGeneratorFactory>
createTupleGeneratorFactoryWithDummyProductShareGenerator(
    int numberOfParty,
    int myId,
    communication::IPartyCommunicationAgentFactory& agentFactory) {
  return std::make_unique<TupleGeneratorFactory>(
      std::unique_ptr<IProductShareGeneratorFactory>(
          std::make_unique<insecure::DummyProductShareGeneratorFactory>(
              agentFactory)),
      std::make_unique<util::AesPrgFactory>(kTestBufferSize),
      kTestBufferSize,
      myId,
      numberOfParty);
}

inline std::unique_ptr<ITupleGeneratorFactory>
createTupleGeneratorFactoryWithRealProductShareGenerator(
    int numberOfParty,
    int myId,
    communication::IPartyCommunicationAgentFactory& agentFactory) {
  auto otFactory = std::make_unique<
      oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory<bool>>(
      myId,
      agentFactory,
      oblivious_transfer::createFerretRcotFactory(
          kTestExtendedSize, kTestBaseSize, kTestWeight));

  auto productShareGeneratorFactory =
      std::unique_ptr<IProductShareGeneratorFactory>(
          std::make_unique<ProductShareGeneratorFactory<bool>>(
              std::make_unique<util::AesPrgFactory>(kTestBufferSize),
              std::move(otFactory)));
  return std::make_unique<TupleGeneratorFactory>(
      std::move(productShareGeneratorFactory),
      std::make_unique<util::AesPrgFactory>(kTestBufferSize),
      kTestBufferSize,
      myId,
      numberOfParty);
}

inline std::unique_ptr<ITupleGeneratorFactory>
createTwoPartyTupleGeneratorFactoryWithDummyRcot(
    int /*numberOfParty*/,
    int myId,
    communication::IPartyCommunicationAgentFactory& agentFactory) {
  auto rcot = std::unique_ptr<
      oblivious_transfer::IRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>());
  return std::make_unique<TwoPartyTupleGeneratorFactory>(
      std::move(rcot),
      std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
          agentFactory),
      myId,
      kTestBufferSize);
}

inline std::unique_ptr<ITupleGeneratorFactory>
createTwoPartyTupleGeneratorFactoryWithRealOt(
    int /*numberOfParty*/,
    int myId,
    communication::IPartyCommunicationAgentFactory& agentFactory) {
  auto rcot = oblivious_transfer::createClassicRcotFactory();
  return std::make_unique<TwoPartyTupleGeneratorFactory>(
      std::move(rcot),
      std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
          agentFactory),
      myId,
      kTestBufferSize);
}

inline std::unique_ptr<ITupleGeneratorFactory>
createTwoPartyTupleGeneratorFactoryWithRcotExtender(
    int /*numberOfParty*/,
    int myId,
    communication::IPartyCommunicationAgentFactory& agentFactory) {
  auto rcot = oblivious_transfer::createFerretRcotFactory(
      kTestExtendedSize, kTestBaseSize, kTestWeight);
  return std::make_unique<TwoPartyTupleGeneratorFactory>(
      std::move(rcot),
      std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
          agentFactory),
      myId,
      kTestBufferSize);
}

} // namespace fbpcf::engine::tuple_generator
