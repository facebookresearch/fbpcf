/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"

namespace fbpcf::engine::communication {

class InMemoryPartyCommunicationAgentHost;

/**
 * This is an in-memory network API. It can be used to send/receive meesages
 * between threads.
 */
class InMemoryPartyCommunicationAgent final : public IPartyCommunicationAgent {
 public:
  InMemoryPartyCommunicationAgent(
      InMemoryPartyCommunicationAgentHost& host,
      int myId)
      : host_{host}, myId_(myId), sentData_(0), receivedData_(0) {}
  /**
   * @inherit doc
   */
  void send(const std::vector<unsigned char>& data) override;

  /**
   * @inherit doc
   */
  std::vector<unsigned char> receive(int size) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {sentData_, receivedData_};
  }

 private:
  InMemoryPartyCommunicationAgentHost& host_;
  int myId_;

  uint64_t sentData_;
  uint64_t receivedData_;
};

/**
 * This object can creates two in memory party communication agent objects that
 * can be used to send/receive messages between two threads. This object is
 * obviously thread-safe.
 */
class InMemoryPartyCommunicationAgentHost {
 public:
  InMemoryPartyCommunicationAgentHost();

  /**
   * Get the communication agent hosted
   * by this object. Each agent can be
   * extracted only once.
   */
  std::unique_ptr<InMemoryPartyCommunicationAgent> getAgent(int Id);

 private:
  /**
   * Allow an in memory communication agent send data to the other.
   */
  void send(int myId, const std::vector<unsigned char>&);

  /**
   * Allow an in memory communication agent receive data from the other.
   */
  std::vector<unsigned char> receive(int myId, int size);

  std::unique_ptr<InMemoryPartyCommunicationAgent> agents_[2];

  // this variable stores all the messages sent. The first is for data sent by
  // party0, the second is for party 1.
  std::queue<std::unique_ptr<std::vector<unsigned char>>> buffers_[2];

  // mutex lock for the two buffers
  std::mutex bufferLock_[2];
  std::condition_variable bufferEmptyVariable_[2];

  friend class InMemoryPartyCommunicationAgent;
};

} // namespace fbpcf::engine::communication
