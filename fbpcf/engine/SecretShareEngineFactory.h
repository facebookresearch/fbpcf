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

#include <fbpcf/engine/tuple_generator/IArithmeticTupleGenerator.h>
#include <fbpcf/engine/tuple_generator/IProductShareGenerator.h>
#include <fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h>
#include "fbpcf/engine/ISecretShareEngineFactory.h"
#include "fbpcf/engine/SecretShareEngine.h"
#include "fbpcf/engine/communication/AgentMapHelper.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/SecretShareEngineCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/ArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/DummyArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/DummyTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/IArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/NullArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/NullTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ProductShareGenerator.h"
#include "fbpcf/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/TupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/TwoPartyTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"

namespace fbpcf::engine {

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
      std::unique_ptr<tuple_generator::IArithmeticTupleGeneratorFactory>
          arithmeticTupleGeneratorFactory,
      communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
      std::unique_ptr<util::IPrgFactory> prgFactoryCreator(),
      int myId,
      int numberOfParty)
      : tupleGeneratorFactory_(std::move(tupleGeneratorFactory)),
        arithmeticTupleGeneratorFactory_(
            std::move(arithmeticTupleGeneratorFactory)),
        communicationAgentFactory_(communicationAgentFactory),
        prgFactoryCreator_(std::move(prgFactoryCreator)),
        myId_(myId),
        numberOfParty_(numberOfParty) {}

  std::unique_ptr<ISecretShareEngine> create() override {
    auto agentMap = communication::getAgentMap(
        numberOfParty_, myId_, communicationAgentFactory_);

    return std::make_unique<SecretShareEngine>(
        tupleGeneratorFactory_->create(),
        arithmeticTupleGeneratorFactory_->create(),
        std::make_unique<communication::SecretShareEngineCommunicationAgent>(
            myId_, std::move(agentMap)),
        prgFactoryCreator_(),
        myId_,
        numberOfParty_);
  }

 private:
  std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
      tupleGeneratorFactory_;
  std::unique_ptr<tuple_generator::IArithmeticTupleGeneratorFactory>
      arithmeticTupleGeneratorFactory_;
  communication::IPartyCommunicationAgentFactory& communicationAgentFactory_;
  std::function<std::unique_ptr<util::IPrgFactory>()> prgFactoryCreator_;
  int myId_;
  int numberOfParty_;
};

inline std::unique_ptr<SecretShareEngineFactory>
getEngineFactoryWithTupleGeneratorFactories(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
        tupleGeneratorFactory,
    std::unique_ptr<tuple_generator::IArithmeticTupleGeneratorFactory>
        arithmeticTupleGeneratorFactory) {
  return std::make_unique<SecretShareEngineFactory>(
      std::move(tupleGeneratorFactory),
      std::move(arithmeticTupleGeneratorFactory),
      communicationAgentFactory,
      []() -> std::unique_ptr<util::IPrgFactory> {
        return std::make_unique<util::AesPrgFactory>();
      },
      myId,
      numberOfParty);
}

/**
 * Creates a secret share engine that supports XOR, NOT, AND operations
 **/
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithBooleanOnlyTupleGenerator(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<tuple_generator::oblivious_transfer::
                        IRandomCorrelatedObliviousTransferFactory> rcotFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  size_t bufferSize = 1600000;

  std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
      tupleGeneratorFactory;
  std::unique_ptr<tuple_generator::IArithmeticTupleGeneratorFactory>
      arithmeticTupleGeneratorFactory;

  if (numberOfParty == 2) {
    tupleGeneratorFactory =
        std::make_unique<tuple_generator::TwoPartyTupleGeneratorFactory>(
            std::move(rcotFactory),
            communicationAgentFactory,
            myId,
            bufferSize,
            metricCollector);
  } else {
    auto biDirectionOtFactory =
        std::make_unique<tuple_generator::oblivious_transfer::
                             RcotBasedBidirectionObliviousTransferFactory>(
            myId, communicationAgentFactory, rcotFactory);

    // use OT for tuple generation
    auto productShareGeneratorFactory =
        std::make_shared<tuple_generator::ProductShareGeneratorFactory>(
            std::make_unique<util::AesPrgFactory>(bufferSize),
            std::move(biDirectionOtFactory));

    tupleGeneratorFactory =
        std::make_unique<tuple_generator::TupleGeneratorFactory>(
            productShareGeneratorFactory,
            std::make_unique<util::AesPrgFactory>(),
            bufferSize,
            myId,
            numberOfParty,
            metricCollector);
  }

  arithmeticTupleGeneratorFactory =
      std::make_unique<tuple_generator::NullArithmeticTupleGeneratorFactory>();

  return getEngineFactoryWithTupleGeneratorFactories(
      myId,
      numberOfParty,
      communicationAgentFactory,
      std::move(tupleGeneratorFactory),
      std::move(arithmeticTupleGeneratorFactory));
}

/**
 * Creates a secret share engine that supports PlUS, NEG, MULT operations
 * If withBooleanTupleGenerator is true, the secret share engine will also
 * support XOR, NOT, AND operations
 */
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithIntegerTupleGenerator(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<tuple_generator::oblivious_transfer::
                        IRandomCorrelatedObliviousTransferFactory> rcotFactory,
    bool withBooleanTupleGenerator,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  size_t bufferSize = 40000;

  std::unique_ptr<tuple_generator::ITupleGeneratorFactory>
      tupleGeneratorFactory;
  std::unique_ptr<tuple_generator::IArithmeticTupleGeneratorFactory>
      arithmeticTupleGeneratorFactory;

  auto biDirectionOtFactory =
      std::make_unique<tuple_generator::oblivious_transfer::
                           RcotBasedBidirectionObliviousTransferFactory>(
          myId, communicationAgentFactory, rcotFactory);

  // use OT for tuple generation
  auto productShareGeneratorFactory =
      std::make_shared<tuple_generator::ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(bufferSize),
          std::move(biDirectionOtFactory));

  arithmeticTupleGeneratorFactory =
      std::make_unique<tuple_generator::ArithmeticTupleGeneratorFactory>(
          productShareGeneratorFactory,
          std::make_unique<util::AesPrgFactory>(),
          bufferSize,
          myId,
          numberOfParty);

  if (withBooleanTupleGenerator) {
    if (numberOfParty == 2) {
      tupleGeneratorFactory =
          std::make_unique<tuple_generator::TwoPartyTupleGeneratorFactory>(
              rcotFactory,
              communicationAgentFactory,
              myId,
              bufferSize,
              metricCollector);
    } else {
      tupleGeneratorFactory =
          std::make_unique<tuple_generator::TupleGeneratorFactory>(
              productShareGeneratorFactory,
              std::make_unique<util::AesPrgFactory>(),
              bufferSize,
              myId,
              numberOfParty,
              metricCollector);
    }
  } else {
    tupleGeneratorFactory =
        std::make_unique<tuple_generator::NullTupleGeneratorFactory>(
            metricCollector);
  }

  return getEngineFactoryWithTupleGeneratorFactories(
      myId,
      numberOfParty,
      communicationAgentFactory,
      std::move(tupleGeneratorFactory),
      std::move(arithmeticTupleGeneratorFactory));
}

/**
 * create a secure engine that utilizes FERRET protocol
 * this function must be called by all parties at the same time since it
 * contains inter-party communication
 */
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithFERRET(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  return getSecureEngineFactoryWithBooleanOnlyTupleGenerator(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createFerretRcotFactory(),
      metricCollector);
}

inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithBooleanAndIntegerTupleGenerator(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  return getSecureEngineFactoryWithIntegerTupleGenerator(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createFerretRcotFactory(),
      true,
      metricCollector);
}

inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithIntegerOnlyTupleGenerator(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  return getSecureEngineFactoryWithIntegerTupleGenerator(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createFerretRcotFactory(),
      false,
      metricCollector);
}
/**
 * create a secure engine that utilizes classic OT protocol
 * this function must be called by all parties at the same time since it
 * contains inter-party commuication
 */
inline std::unique_ptr<SecretShareEngineFactory>
getSecureEngineFactoryWithClassicOt(
    int myId,
    int numberOfParty,
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  return getSecureEngineFactoryWithBooleanOnlyTupleGenerator(
      myId,
      numberOfParty,
      communicationAgentFactory,
      tuple_generator::oblivious_transfer::createClassicRcotFactory(),
      metricCollector);
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
    communication::IPartyCommunicationAgentFactory& communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
  return getEngineFactoryWithTupleGeneratorFactories(
      myId,
      numberOfParty,
      communicationAgentFactory,
      std::make_unique<tuple_generator::insecure::DummyTupleGeneratorFactory>(
          metricCollector),
      std::make_unique<
          tuple_generator::insecure::DummyArithmeticTupleGeneratorFactory>());
}

} // namespace fbpcf::engine
