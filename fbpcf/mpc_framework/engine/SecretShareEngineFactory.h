/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "fbpcf/mpc_framework/engine/ISecretShareEngineFactory.h"
#include "fbpcf/mpc_framework/engine/SecretShareEngine.h"
#include "fbpcf/mpc_framework/engine/communication/AgentMapHelper.h"
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/engine/communication/SecretShareEngineCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/DummyTupleGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ProductShareGenerator.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/TupleGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/TwoPartyTupleGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::mpc_framework::engine {

/**
 * This factory creates a secure secret share MPC engine, provided underlying
 * factories create secure components.
 * The security assumption for this engine is semi-honest. Therefore all the
 * underlying components have to be at least COMPOSABLY secure against
 * semi-honest adversaries (composably secure effectively means it can be used
 * as a blackbox). A side note: none of these components address any security
 * concerns outside the world of MPC. Therefore threats like man-in-the-middle
 * need to be addressed with other methods separately.
 */
class SecretShareEngineFactory final : public ISecretShareEngineFactory {
 public:
  SecretShareEngineFactory(
      std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
          tupleGeneratorFactory,
      communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
      std::unique_ptr<util::IPrgFactory> prgFactoryCreator(),
      int myId,
      int numberOfParty)
      : tupleGeneratorFactory_(std::move(tupleGeneratorFactory)),
        communicationAgentFactory_(communicationAgentFactory),
        prgFactoryCreator_(std::move(prgFactoryCreator)),
        myId_(myId),
        numberOfParty_(numberOfParty) {}

  std::unique_ptr<ISecretShareEngine> create() override {
    auto agentMap = communication::getAgentMap(
        numberOfParty_, myId_, communicationAgentFactory_);

    return std::make_unique<SecretShareEngine>(
        tupleGeneratorFactory_->create(),
        std::make_unique<communication::SecretShareEngineCommunicationAgent>(
            myId_, std::move(agentMap)),
        prgFactoryCreator_(),
        myId_,
        numberOfParty_);
  }

 private:
  std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
      tupleGeneratorFactory_;
  communication::IPartyCommunicationAgentFactory& communicationAgentFactory_;
  std::function<std::unique_ptr<util::IPrgFactory>()> prgFactoryCreator_;
  int myId_;
  int numberOfParty_;
};

inline std::unique_ptr<SecretShareEngineFactory>
getEngineFactoryWithTupleGeneratorFactory(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
        tupleGeneratorFactory) {
  return std::make_unique<SecretShareEngineFactory>(
      std::move(tupleGeneratorFactory),
      communicationAgentFactory,
      []() -> std::unique_ptr<util::IPrgFactory> {
        return std::make_unique<util::AesPrgFactory>();
      },
      myId,
      numberOfParty);
}

template <class T>
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithRcotFactory(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::unique_ptr<tuple_generator::oblivious_transfer::
                        IRandomCorrelatedObliviousTransferFactory>
        rcotFactory) {
  size_t bufferSize = 1600000;

  std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
      tupleGeneratorFactory;

  if (numberOfParty == 2) {
    tupleGeneratorFactory =
        std::make_unique<tuple_generator::TwoPartyTupleGeneratorFactory>(
            std::move(rcotFactory),
            communicationAgentFactory,
            myId,
            bufferSize);
  } else {
    auto biDirectionOtFactory =
        std::make_unique<tuple_generator::oblivious_transfer::
                             RcotBasedBidirectionObliviousTransferFactory<T>>(
            myId, communicationAgentFactory, std::move(rcotFactory));

    // use OT for tuple generation
    auto productShareGeneratorFactory =
        std::make_unique<tuple_generator::ProductShareGeneratorFactory<T>>(
            std::make_unique<util::AesPrgFactory>(bufferSize),
            std::move(biDirectionOtFactory));

    tupleGeneratorFactory =
        std::make_unique<tuple_generator::TupleGeneratorFactory>(
            std::move(productShareGeneratorFactory),
            std::make_unique<util::AesPrgFactory>(),
            bufferSize,
            myId,
            numberOfParty);
  }

  return getEngineFactoryWithTupleGeneratorFactory(
      myId,
      numberOfParty,
      communicationAgentFactory,
      std::move(tupleGeneratorFactory));
}
/**
 * create a secure engine that utilizes FERRET protocol
 * this function must be called by all parties at the same time since it
 * contains inter-party communication
 */
template <class T>
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithFERRET(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory) {
  return getSecureEngineFactoryWithRcotFactory<T>(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createFerretRcotFactory());
}

/**
 * create a secure engine that utilizes classic OT protocol
 * this function must be called by all parties at the same time since it
 * contains inter-party commuication
 */
template <class T>
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithClassicOt(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory) {
  return getSecureEngineFactoryWithRcotFactory<T>(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createClassicRcotFactory());
}

/**
 * This API should be used in test only!
 * create a secure engine that use a dummy tuple generator.
 * this function must be called by all parties at the same time since it
 * contains inter-party commuication.
 */
inline std::unique_ptr<SecretShareEngineFactory>
getInsecureEngineFactoryWithDummyTupleGenerator(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory) {
  return getEngineFactoryWithTupleGeneratorFactory(
      myId,
      numberOfParty,
      communicationAgentFactory,
      std::make_unique<
          tuple_generator::insecure::DummyTupleGeneratorFactory>());
}

} // namespace fbpcf::mpc_framework::engine
