/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/DummySecretShareEngine.h"
#include "fbpcf/engine/ISecretShareEngineFactory.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"

namespace fbpcf::engine {

class DummySecretShareEngineFactory final : public ISecretShareEngineFactory {
 public:
  explicit DummySecretShareEngineFactory(int myId) : myId_(myId) {}

  std::unique_ptr<ISecretShareEngine> create() override {
    return std::make_unique<DummySecretShareEngine>(myId_);
  }

 private:
  int myId_;
};

/**
 * This API is used in test only.
 */
inline std::unique_ptr<DummySecretShareEngineFactory> getDummyEngineFactory(
    int myId,
    [[maybe_unused]] int numberOfParty,
    [[maybe_unused]] communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  return std::make_unique<DummySecretShareEngineFactory>(myId);
}

} // namespace fbpcf::engine
