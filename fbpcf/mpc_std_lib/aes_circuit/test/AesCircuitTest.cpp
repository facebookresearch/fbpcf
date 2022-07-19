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
#include <cstdint>
#include <future>
#include <memory>
#include <random>
#include <unordered_map>

#define AES_CIRCUIT_TEST_FRIENDS                         \
  friend class AesCircuitSBoxTestSuite;                  \
  FRIEND_TEST(AesCircuitSBoxTestSuite, SBoxInplaceTest); \
  FRIEND_TEST(AesCircuitSBoxTestSuite, InverseSBoxInplaceTest);

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit_impl.h"
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

void intToBinaryArray(unsigned int n, std::array<bool, 8>& array) {
  for (unsigned int i = 0; i != 8; ++i) {
    array[7 - i] = n & 1;
    n = n >> 1;
  }
}

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

class AesCircuitSBoxTestSuite
    : public ::testing::TestWithParam<std::tuple<uint8_t, uint8_t>> {};

TEST_P(AesCircuitSBoxTestSuite, SBoxInplaceTest) {
  AesCircuit<bool> aes;
  std::array<bool, 8> result;
  std::array<bool, 8> expectedResult;
  uint8_t input = std::get<0>(GetParam());
  uint8_t expectedOutput = std::get<1>(GetParam());
  intToBinaryArray(input, result);
  intToBinaryArray(expectedOutput, expectedResult);

  aes.sBoxInPlace(result);

  testArrayEq(expectedResult, result);
}

TEST_P(AesCircuitSBoxTestSuite, InverseSBoxInplaceTest) {
  AesCircuit<bool> aes;
  std::array<bool, 8> result;
  std::array<bool, 8> expectedResult;
  uint8_t input = std::get<1>(GetParam());
  uint8_t expectedOutput = std::get<0>(GetParam());
  intToBinaryArray(input, result);
  intToBinaryArray(expectedOutput, expectedResult);

  aes.inverseSBoxInPlace(result);

  testArrayEq(expectedResult, result);
}

INSTANTIATE_TEST_SUITE_P(
    SBoxTests,
    AesCircuitSBoxTestSuite,
    ::testing::Values(
        std::make_tuple(0x00, 0x63),
        std::make_tuple(0x11, 0x82),
        std::make_tuple(0x22, 0x93),
        std::make_tuple(0x33, 0xC3),
        std::make_tuple(0x44, 0x1B),
        std::make_tuple(0x55, 0xFC),
        std::make_tuple(0x66, 0x33),
        std::make_tuple(0x77, 0xF5),
        std::make_tuple(0x88, 0xC4),
        std::make_tuple(0x99, 0xEE),
        std::make_tuple(0xAA, 0xAC),
        std::make_tuple(0xBB, 0xEA),
        std::make_tuple(0xCC, 0x4B),
        std::make_tuple(0xDD, 0xC1),
        std::make_tuple(0xEE, 0x28),
        std::make_tuple(0xFF, 0x16)));

} // namespace fbpcf::mpc_std_lib::aes_circuit
