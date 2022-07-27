/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/AgentMapHelper.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/scheduler/EagerScheduler.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/LazyScheduler.h"
#include "fbpcf/scheduler/NetworkPlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/GateKeeper.h"

namespace fbpcf::scheduler {

template <bool unsafe>
inline std::unique_ptr<IArithmeticScheduler> createPlaintextScheduler(
    int /*myId*/,
    engine::communication::IPartyCommunicationAgentFactory&
    /*communicationAgentFactory*/) {
  return std::make_unique<PlaintextScheduler>(
      WireKeeper::createWithVectorArena<unsafe>());
}

template <bool unsafe>
inline std::unique_ptr<IArithmeticScheduler> createNetworkPlaintextScheduler(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto numberOfParties = 2;
  auto agentMap = engine::communication::getAgentMap(
      numberOfParties, myId, communicationAgentFactory);

  return std::make_unique<NetworkPlaintextScheduler>(
      myId, std::move(agentMap), WireKeeper::createWithVectorArena<unsafe>());
}

// this function creates a eager scheduler with real secure engine
inline std::unique_ptr<IScheduler> createEagerSchedulerWithRealEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getSecureEngineFactoryWithFERRET<bool>(
      myId, 2, communicationAgentFactory);

  return std::make_unique<EagerScheduler>(
      engineFactory->create(),
      WireKeeper::createWithVectorArena</*unsafe*/ true>());
}

// this function creates a lazy scheduler with real secure engine
inline std::unique_ptr<IScheduler> createLazySchedulerWithRealEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getSecureEngineFactoryWithFERRET<bool>(
      myId, 2, communicationAgentFactory);

  std::shared_ptr<IWireKeeper> wireKeeper =
      WireKeeper::createWithVectorArena</*unsafe*/ true>();

  return std::make_unique<LazyScheduler>(
      engineFactory->create(),
      wireKeeper,
      std::make_unique<GateKeeper>(wireKeeper));
}

inline std::unique_ptr<IScheduler> createEagerSchedulerWithClassicOT(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getSecureEngineFactoryWithClassicOt<bool>(
      myId, 2, communicationAgentFactory);

  return std::make_unique<EagerScheduler>(
      engineFactory->create(),
      WireKeeper::createWithVectorArena</*unsafe*/ true>());
}

inline std::unique_ptr<IScheduler> createLazySchedulerWithClassicOT(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getSecureEngineFactoryWithClassicOt<bool>(
      myId, 2, communicationAgentFactory);

  std::shared_ptr<IWireKeeper> wireKeeper =
      WireKeeper::createWithVectorArena</*unsafe*/ true>();

  return std::make_unique<LazyScheduler>(
      engineFactory->create(),
      wireKeeper,
      std::make_unique<GateKeeper>(wireKeeper));
}

// this function creates a eager scheduler with insecure engine
template <bool unsafe>
inline std::unique_ptr<IScheduler> createEagerSchedulerWithInsecureEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getInsecureEngineFactoryWithDummyTupleGenerator(
      myId, 2, communicationAgentFactory);

  return std::make_unique<EagerScheduler>(
      engineFactory->create(), WireKeeper::createWithVectorArena<unsafe>());
}

// this function creates a lazy scheduler with insecure engine
template <bool unsafe>
inline std::unique_ptr<IScheduler> createLazySchedulerWithInsecureEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory) {
  auto engineFactory = engine::getInsecureEngineFactoryWithDummyTupleGenerator(
      myId, 2, communicationAgentFactory);

  std::shared_ptr<IWireKeeper> wireKeeper =
      WireKeeper::createWithVectorArena<unsafe>();

  return std::make_unique<LazyScheduler>(
      engineFactory->create(),
      wireKeeper,
      std::make_unique<GateKeeper>(wireKeeper));
}

} // namespace fbpcf::scheduler
