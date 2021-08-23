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

#include <emp-sh2pc/emp-sh2pc.h>

#include "folly/Synchronized.h"

#include "EmpGame.h"
#include "QueueIO.h"
#include "fbpcf/system/CpuUtil.h"

namespace fbpcf::mpc {
template <class EmpGameClass, class InputDataType, class OutputDataType>
std::pair<OutputDataType, OutputDataType> test(
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
      exit(EXIT_FAILURE);
    }
  };

  auto futureAlice = std::async(lambda, Party::Alice, aliceInput);
  auto futureBob = std::async(lambda, Party::Bob, bobInput);

  auto resAlice = futureAlice.get();
  auto resBob = futureBob.get();

  return std::make_pair(resAlice, resBob);
}

template <class TestCase>
void wrapTestWithParty(TestCase testCase) {
  auto queueA = std::make_shared<folly::Synchronized<std::queue<char>>>();
  auto queueB = std::make_shared<folly::Synchronized<std::queue<char>>>();

  auto lambda = [&queueA, &queueB, &testCase](Party party) {
    auto io = std::make_unique<QueueIO>(
        party == Party::Alice ? QueueIO{queueA, queueB}
                              : QueueIO{queueB, queueA});
    emp::setup_semi_honest(io.get(), static_cast<int>(party));

    try {
      testCase(party);
    } catch (const std::exception& e) {
      std::cout << "Exception occured. " << e.what() << endl;
      exit(EXIT_FAILURE);
    }
  };

  auto futureAlice = std::async(lambda, Party::Alice);
  auto futureBob = std::async(lambda, Party::Bob);

  futureAlice.wait();
  futureBob.wait();
}

template <class TestCase>
void wrapTest(TestCase testCase) {
  return wrapTestWithParty<std::function<void(Party party)>>(
      [&testCase](Party party) { return testCase(); });
}

bool isTestable();
} // namespace fbpcf::mpc
