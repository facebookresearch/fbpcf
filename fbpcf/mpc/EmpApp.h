/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <emp-sh2pc/emp-sh2pc.h>

#include <folly/logging/xlog.h>

#include "EmpGame.h"

namespace fbpcf {
template <class GameType, class InputDataType, class OutputDataType>
class EmpApp {
 public:
  EmpApp(
      Party party,
      const std::string& serverIp,
      uint16_t port,
      bool useTls = false,
      const std::string& tlsDir = "")
      : party_{party},
        serverIp_{serverIp},
        port_{port},
        useTls_{useTls},
        tlsDir_{tlsDir} {}

  virtual ~EmpApp(){};

  virtual void run() {
    auto io = std::make_unique<emp::NetIO>(
        party_ == Party::Alice ? nullptr : serverIp_.c_str(), port_);

    auto inputData = getInputData();

    GameType game{std::move(io), party_};
    auto output = game.perfPlay(inputData);

    putOutputData(output);
  };

 protected:
  virtual InputDataType getInputData() = 0;
  virtual void putOutputData(const OutputDataType& output) = 0;

 protected:
  Party party_; // Alice or Bob (Publisher or Partner)
  std::string serverIp_; // Ip address of the publisher
  uint16_t port_; // Port to bind to, or port to connect to (for publisher or
                  // partner respectively)
  bool useTls_; // whether to use TLS for communication
  const std::string& tlsDir_; // directory that holds the TLS certificates/keys
};
} // namespace fbpcf
