/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <fbpcf/primitive/mac/IMacFactory.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <random>
#include <vector>
#include "fbpcf/engine/util/util.h"
#include "fbpcf/primitive/mac/AesCmacFactory.h"
#include "fbpcf/primitive/mac/S2vFactory.h"

namespace fbpcf::primitive::mac {

std::vector<unsigned char> generateRandomText(size_t byteNum) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<unsigned char> dist(0, 0xFF);
  std::vector<unsigned char> randomText(byteNum);
  for (size_t j = 0; j < byteNum; ++j) {
    randomText[j] = dist(e);
  }
  return randomText;
}

void testGetMacM128i(std::unique_ptr<IMacFactory> macFactory) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<size_t> dist(1, 2048);
  for (size_t i = 0; i < 128; ++i) {
    size_t byteNum = dist(e);
    const std::vector<unsigned char> randomText1 = generateRandomText(byteNum);
    const std::vector<unsigned char> randomText2 = generateRandomText(byteNum);
    std::vector<unsigned char> randomKey1 = generateRandomText(16);
    std::vector<unsigned char> randomKey2 = generateRandomText(16);

    // same key and same text will give the same mac
    const auto mac1 = macFactory->create(randomKey1);
    const auto mac2 = macFactory->create(randomKey1);
    const __m128i macRes1 = mac1->getMacM128i(randomText1);
    const __m128i macRes2 = mac2->getMacM128i(randomText1);
    EXPECT_EQ(_mm_extract_epi64(macRes1, 0), _mm_extract_epi64(macRes2, 0));
    EXPECT_EQ(_mm_extract_epi64(macRes1, 1), _mm_extract_epi64(macRes2, 1));

    // different key and same text will give the different mac
    const auto mac3 = macFactory->create(randomKey2);
    const __m128i macRes3 = mac3->getMacM128i(randomText1);
    // The following check should be satisfactory as it is highly improbable
    // that half of two Mac are equal.
    EXPECT_NE(_mm_extract_epi64(macRes1, 0), _mm_extract_epi64(macRes3, 0));
    EXPECT_NE(_mm_extract_epi64(macRes1, 1), _mm_extract_epi64(macRes3, 1));

    // same key and different text will give the different mac
    const __m128i macRes4 = mac2->getMacM128i(randomText2);
    // The following check should be satisfactory as it is highly improbable
    // that half of two Mac are equal.
    EXPECT_NE(_mm_extract_epi64(macRes1, 0), _mm_extract_epi64(macRes4, 0));
    EXPECT_NE(_mm_extract_epi64(macRes1, 1), _mm_extract_epi64(macRes4, 1));
  }
}

void testGetMac128(std::unique_ptr<IMacFactory> macFactory) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<size_t> dist(1, 2048);
  for (size_t i = 0; i < 128; ++i) {
    size_t byteNum = dist(e);
    const std::vector<unsigned char> randomText1 = generateRandomText(byteNum);
    const std::vector<unsigned char> randomText2 = generateRandomText(byteNum);
    std::vector<unsigned char> randomKey1 = generateRandomText(16);
    std::vector<unsigned char> randomKey2 = generateRandomText(16);

    // same key and same text will give the same mac
    const auto mac1 = macFactory->create(randomKey1);
    const auto mac2 = macFactory->create(randomKey1);
    const std::vector<unsigned char> macRes1 = mac1->getMac128(randomText1);
    const std::vector<unsigned char> macRes2 = mac2->getMac128(randomText1);
    EXPECT_EQ(macRes1, macRes2);

    // different key and same text will give the different mac
    auto mac3 = macFactory->create(randomKey2);
    const std::vector<unsigned char> macRes3 = mac3->getMac128(randomText1);
    EXPECT_NE(macRes1, macRes3);

    // same key and different text will give the different mac
    const std::vector<unsigned char> macRes4 = mac2->getMac128(randomText2);
    EXPECT_NE(macRes1, macRes4);
  }
}

void testConsistency(std::unique_ptr<IMacFactory> macFactory) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<size_t> dist(1, 2048);
  for (size_t i = 0; i < 128; ++i) {
    size_t blockNum = dist(e);
    const std::vector<unsigned char> randomText =
        generateRandomText(blockNum * 16);
    std::vector<unsigned char> randomKey = generateRandomText(16);
    std::vector<__m128i> randomTextM128(blockNum);
    for (size_t j = 0; j < randomTextM128.size(); ++j) {
      std::vector<unsigned char> randomBlock(
          randomText.begin() + j * 16, randomText.begin() + (j + 1) * 16);
      randomTextM128[j] = engine::util::buildM128i(randomBlock);
    }
    const auto mac = macFactory->create(randomKey);
    const __m128i macRes1 = mac->getMacM128i(randomText);
    const __m128i macRes2 =
        engine::util::buildM128i(mac->getMac128(randomText));
    EXPECT_EQ(_mm_extract_epi64(macRes1, 0), _mm_extract_epi64(macRes2, 0));
    EXPECT_EQ(_mm_extract_epi64(macRes1, 1), _mm_extract_epi64(macRes2, 1));
  }
}

void testEmptyInput(std::unique_ptr<IMacFactory> macFactory) {
  std::vector<unsigned char> randomKey = generateRandomText(16);
  const std::vector<unsigned char> emptyText(0);
  std::vector<__m128i> emptyTextM128(0);

  const auto mac = macFactory->create(randomKey);

  const __m128i macRes1 = mac->getMacM128i(emptyText);
  const __m128i macRes2 = engine::util::buildM128i(mac->getMac128(emptyText));
  EXPECT_EQ(_mm_extract_epi64(macRes1, 0), _mm_extract_epi64(macRes2, 0));
  EXPECT_EQ(_mm_extract_epi64(macRes1, 1), _mm_extract_epi64(macRes2, 1));
}

TEST(AesCMacTest, testGetMacM128i) {
  testGetMacM128i(std::make_unique<AesCmacFactory>());
}

TEST(AesCMacTest, testGetMac128) {
  testGetMac128(std::make_unique<AesCmacFactory>());
}

TEST(AesCMacTest, testConsistency) {
  testConsistency(std::make_unique<AesCmacFactory>());
}

TEST(AesCMacTest, testEmptyInput) {
  testEmptyInput(std::make_unique<AesCmacFactory>());
}

TEST(S2vTest, testGetMacM128i) {
  testGetMacM128i(std::make_unique<S2vFactory>());
}

TEST(S2vTest, testGetMac128) {
  testGetMac128(std::make_unique<S2vFactory>());
}

TEST(S2vTest, testConsistency) {
  testConsistency(std::make_unique<S2vFactory>());
}

TEST(S2vTest, testEmptyInput) {
  testEmptyInput(std::make_unique<S2vFactory>());
}
} // namespace fbpcf::primitive::mac
