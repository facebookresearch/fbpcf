/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include "fbpcf/engine/SecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/test/TupleGeneratorTestHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/aes.h"

namespace fbpcf::engine {

class SecretShareEngineTestHelper {
 public:
  std::vector<std::unique_ptr<ISecretShareEngine>>
  createEnginesWithDummyTupleGenerator(int8_t numberOfParties) {
    return createEngines(
        numberOfParties, tuple_generator::createDummyTupleGeneratorFactory);
  }

  std::vector<std::unique_ptr<ISecretShareEngine>>
  createEnginesWithDummyProductShareGenerator(int8_t numberOfParties) {
    return createEngines(
        numberOfParties,
        tuple_generator::
            createInMemoryTupleGeneratorFactoryWithDummyProductShareGenerator);
  }

  std::vector<std::unique_ptr<ISecretShareEngine>>
  createEnginesWithRealProductShareGenerator(int8_t numberOfParties) {
    return createEngines(
        numberOfParties,
        tuple_generator::
            createInMemoryTupleGeneratorFactoryWithRealProductShareGenerator);
  }

 private:
  using TupleGeneratorFactoryCreator =
      std::unique_ptr<tuple_generator::ITupleGeneratorFactory>(
          int numberOfParty,
          int myId,
          communication::IPartyCommunicationAgentFactory& agentFactory);

  std::vector<std::unique_ptr<ISecretShareEngine>> createEngines(
      int8_t numberOfParties,
      TupleGeneratorFactoryCreator tupleGeneratorFactoryCreator) {
    auto factories = communication::getInMemoryAgentFactory(numberOfParties);
    factories_.insert(
        factories_.end(),
        std::make_move_iterator(factories.begin()),
        std::make_move_iterator(factories.end()));

    // The secret share engines have to be created as futures because the
    // parties need to exchange keys as part of initialization.
    std::vector<std::future<std::unique_ptr<ISecretShareEngine>>> futures;
    for (auto i = 0; i < numberOfParties; ++i) {
      futures.push_back(std::async(
          [i, numberOfParties](
              TupleGeneratorFactoryCreator tupleGeneratorFactoryCreator,
              std::reference_wrapper<
                  communication::IPartyCommunicationAgentFactory> agentFactory)
              -> std::unique_ptr<ISecretShareEngine> {
            auto factory = std::make_unique<SecretShareEngineFactory>(
                tupleGeneratorFactoryCreator(numberOfParties, i, agentFactory),
                agentFactory,
                []() -> std::unique_ptr<util::IPrgFactory> {
                  return std::make_unique<util::AesPrgFactory>();
                },
                i,
                numberOfParties);
            return factory->create();
          },
          tupleGeneratorFactoryCreator,
          std::reference_wrapper<
              communication::IPartyCommunicationAgentFactory>(
              *factories_.at(factories_.size() - numberOfParties + i))));
    }

    std::vector<std::unique_ptr<ISecretShareEngine>> engines;
    for (auto i = 0; i < numberOfParties; ++i) {
      engines.push_back(futures.at(i).get());
    }
    return engines;
  }

  // These need to be stored as members so the communication agents
  // are not destroyed after creating the engines.
  std::vector<std::unique_ptr<communication::IPartyCommunicationAgentFactory>>
      factories_;
};

} // namespace fbpcf::engine
