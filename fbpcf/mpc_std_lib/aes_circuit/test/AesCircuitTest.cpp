/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <future>
#include <memory>
#include <random>
#include <unordered_map>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit.h"
#include "fbpcf/mpc_std_lib/aes_circuit/DummyAesCircuitFactory.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuit.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
class AesCircuitTests : public AesCircuit<BitType> {
 public:
  void testShiftRowInPlace(std::vector<bool> plaintext) {
    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 3);
    int row = dist(e);

    std::array<std::array<bool, 8>, 4> word;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 8; j++) {
        word[i][j] = plaintext[32 * i + 8 * row + j];
      }
    }

    AesCircuit<bool>::shiftRowInPlace(word, row);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 8; j++) {
        EXPECT_EQ(word[i][j], plaintext[32 * ((i + row) % 4) + 8 * row + j]);
      }
    }
  }
};

std::vector<bool> generateRandomPlaintext(int size = 128) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  std::vector<bool> plaintext(size);
  for (size_t i = 0; i < size; i++) {
    plaintext[i] = dist(e);
  }
  return plaintext;
}

void testDummyAesCircuitInPlaintext(
    std::shared_ptr<insecure::DummyAesCircuitFactory<bool>>
        DummyAesCircuitFactory) {
  auto DummyAesCircuit = DummyAesCircuitFactory->create();
  auto plaintext = generateRandomPlaintext();
  std::vector<bool> dummyKey(11 * 16 * 8);
  auto rst1 = DummyAesCircuit->encrypt(plaintext, dummyKey);
  testVectorEq(rst1, plaintext);
  auto rst2 = DummyAesCircuit->decrypt(rst1, dummyKey);
  testVectorEq(rst2, rst1);
}

TEST(AesCircuitTest, testDummyAesCircuit) {
  testDummyAesCircuitInPlaintext(
      std::make_unique<insecure::DummyAesCircuitFactory<bool>>());
}

TEST(AesCircuitTest, testShiftRowInPlace) {
  auto plaintext = generateRandomPlaintext();
  AesCircuitTests<bool> test;
  test.testShiftRowInPlace(plaintext);
}

} // namespace fbpcf::mpc_std_lib::aes_circuit
