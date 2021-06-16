/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <exception>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "folly/Synchronized.h"

#include "../system/CpuUtil.h"
#include "EmpGame.h"
#include "QueueIO.h"

namespace pcf::mpc {
template <class EmpGameClass, class InputDataType, class OutputDataType>
std::pair<OutputDataType, OutputDataType> testGame(
    InputDataType aliceInput,
    InputDataType bobInput) {
  auto queueA = std::make_shared<folly::Synchronized<std::queue<char>>>();
  auto queueB = std::make_shared<folly::Synchronized<std::queue<char>>>();

  auto lambda = [&queueA, &queueB](Party party, InputDataType input) {
    auto io = std::make_unique<QueueIO>(
        party == Party::Alice ? QueueIO{queueA, queueB}
                              : QueueIO{queueB, queueA});
    EmpGameClass game{std::move(io), party};

    try {
      return game.perfPlay(input);
    } catch (const std::exception& e) {
      std::cout << "Exception occured. " << e.what() << endl;
      exit(-1);
    }
  };

  auto futureAlice = std::async(lambda, Party::Alice, aliceInput);
  auto futureBob = std::async(lambda, Party::Bob, bobInput);

  auto resAlice = futureAlice.get();
  auto resBob = futureBob.get();

  return std::make_pair(resAlice, resBob);
}

// Used to test an arbitrary function using emp data types, since
// those cannot be constructed outside the context of an EmpGame.
template <typename F>
void testFunction(F f) {
  class FunctionTest : public EmpGame<QueueIO, F, bool> {
   public:
    FunctionTest(std::unique_ptr<QueueIO> io, Party party)
        : EmpGame<QueueIO, F, bool>(std::move(io), party) {}

    bool play(const F& f) override {
      f();
      return true;
    }
  };

  testGame<FunctionTest, F, bool>(f, f);
}

bool isTestable();
} // namespace pcf::mpc
