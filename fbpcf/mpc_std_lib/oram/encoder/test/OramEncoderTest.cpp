/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <memory>
#include <random>

#include "fbpcf/test/TestHelper.h"

#include "fbpcf/mpc_std_lib/oram/encoder/IOramEncoder.h"
#include "fbpcf/mpc_std_lib/oram/encoder/OramDecoder.h"
#include "fbpcf/mpc_std_lib/oram/encoder/OramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

void testDecoderValidity(
    std::unique_ptr<IOramEncoder> encoder,
    const std::vector<uint32_t>& breakdownMapping,
    const std::vector<std::vector<uint32_t>>& expectedBreakdownValues,
    std::optional<uint32_t> filterIndex) {
  auto mappingConfig = encoder->exportMappingConfig();
  OramDecoder decoder(std::move(mappingConfig));

  auto decodedTuples = decoder.decodeORAMIndexes(breakdownMapping);

  EXPECT_EQ(breakdownMapping.size(), decodedTuples.size());

  for (int i = 0; i < decodedTuples.size(); i++) {
    if (filterIndex.has_value() && breakdownMapping[i] == filterIndex.value()) {
      EXPECT_EQ(decodedTuples[i], std::vector<uint32_t>(0));
    } else {
      EXPECT_EQ(decodedTuples[i], expectedBreakdownValues[i]);
    }
  }
}

TEST(OramEncoderTest, TestEncoderNoFilters) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  std::unique_ptr<IOramEncoder> encoder =
      std::make_unique<OramEncoder>(std::move(filters));

  /*
   * Total breakdown columns: 4
   * B1: [0 - 1]
   * B2: [0 - 2]
   * B3: [0 - 1]
   * B4: [0 - 3]
   * Total possible Breakdowns: 1 + 2 * 3 * 2 * 4 = 49
   * i = b1 * 24 + b2 * 8 + b3 * 4 + b4 + 1
   */

  std::vector<std::vector<uint32_t>> breakdownTuples(0);
  for (uint32_t i = 0; i < 48; i++) {
    std::vector<uint32_t> breakdownValues{
        i / 24, (i / 8) % 3, (i / 4) % 2, i % 4};
    breakdownTuples.push_back(breakdownValues);
  }

  auto mapping = encoder->generateORAMIndexes(breakdownTuples);

  for (int i = 0; i < 48; i++) {
    EXPECT_EQ(mapping[i], i);
  }

  breakdownTuples.clear();

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomB1AndB3(0, 1);
  std::uniform_int_distribution<uint32_t> randomB2(0, 2);
  std::uniform_int_distribution<uint32_t> randomB4(0, 3);

  for (int i = 0; i < 100; i++) {
    std::vector<uint32_t> breakdownValues{
        randomB1AndB3(e), randomB2(e), randomB1AndB3(e), randomB4(e)};
    breakdownTuples.push_back(breakdownValues);
  }

  mapping = encoder->generateORAMIndexes(breakdownTuples);

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(
        mapping[i],
        breakdownTuples[i][0] * 24 + breakdownTuples[i][1] * 8 +
            breakdownTuples[i][2] * 4 + breakdownTuples[i][3]);
  }

  testDecoderValidity(
      std::move(encoder), mapping, breakdownTuples, std::nullopt);
}

void encoderWithFiltersTest(
    std::unique_ptr<std::vector<std::unique_ptr<IFilter>>> filters,
    const std::vector<uint32_t>& expected,
    std::optional<uint32_t> filterIndex) {
  /*
   * Total breakdown columns: 2
   * B1: [0 - 9]
   * B2: [0 - 9]
   * i = b1 * 10 + b2 (before filtering)
   */

  std::vector<std::vector<uint32_t>> breakdownTuples(0);
  for (uint32_t i = 0; i < 100; i++) {
    std::vector<uint32_t> breakdownValues{i / 10, i % 10};
    breakdownTuples.push_back(breakdownValues);
  }

  std::unique_ptr<IOramEncoder> encoder =
      std::make_unique<OramEncoder>(std::move(filters));

  auto mapping = encoder->generateORAMIndexes(breakdownTuples);

  testVectorEq(expected, mapping);

  testDecoderValidity(
      std::move(encoder), mapping, breakdownTuples, filterIndex);
}

TEST(OramEncoderTest, EmptyFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);

  std::vector<uint32_t> expected(0);

  for (size_t i = 0; i < 100; i++) {
    expected.push_back(i);
  }

  encoderWithFiltersTest(std::move(filters), expected, std::nullopt);
}

TEST(OramEncoderTest, GTFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 0, 4));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 1, 4));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 > 4 && b2 > 4) {
      expected.push_back((b1 - 5) * 5 + (b2 - 5) + 1);
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

TEST(OramEncoderTest, LTFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::LT, 1, 5));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::LT, 0, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 < 5 && b2 < 5) {
      expected.push_back(b1 * 5 + b2 + (b1 != 0 ? 1 : 0));
    } else {
      expected.push_back(5);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 5);
}

TEST(OramEncoderTest, GTEFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GTE, 0, 4));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GTE, 1, 4));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 >= 4 && b2 >= 4) {
      expected.push_back((b1 - 4) * 6 + (b2 - 4) + 1);
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

TEST(OramEncoderTest, LTEFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::LTE, 1, 5));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::LTE, 0, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 <= 5 && b2 <= 5) {
      expected.push_back(b1 * 6 + b2 + (b1 != 0 ? 1 : 0));
    } else {
      expected.push_back(6);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 6);
}

TEST(OramEncoderTest, EQFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::EQ, 0, 3));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 1, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 == 3 && b2 > 5) {
      expected.push_back(1 + (b2 - 6));
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

TEST(OramEncoderTest, NEQFilterTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::NEQ, 0, 0));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 1, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (b1 != 0 && b2 > 5) {
      expected.push_back(1 + (b1 - 1) * 4 + (b2 - 6));
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

TEST(OramEncoderTest, SubsetOfTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  std::vector<uint32_t> filterValues{0, 2, 5};
  filters->push_back(
      std::make_unique<VectorValueFilter>(IFilter::SUBSET_OF, 0, filterValues));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 1, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if ((b1 == 0 || b1 == 2 || b1 == 5) && b2 > 5) {
      expected.push_back(1 + (b1 == 0 ? 0 : b1 == 2 ? 1 : 2) * 4 + (b2 - 6));
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

TEST(OramEncoderTest, NotSubsetOfTest) {
  auto filters = std::make_unique<std::vector<std::unique_ptr<IFilter>>>(0);
  std::vector<uint32_t> filterValues{0, 2, 5};
  filters->push_back(std::make_unique<VectorValueFilter>(
      IFilter::NOT_SUBSET_OF, 0, filterValues));
  filters->push_back(std::make_unique<SingleValueFilter>(IFilter::GT, 1, 5));

  std::vector<uint32_t> expected(0);
  for (uint32_t i = 0; i < 100; i++) {
    uint32_t b1 = i / 10;
    uint32_t b2 = i % 10;

    if (!(b1 == 0 || b1 == 2 || b1 == 5) && b2 > 5) {
      expected.push_back(
          1 +
          (b1 == 1      ? 0
               : b1 < 5 ? b1 - 2
                        : b1 - 3) *
              4 +
          (b2 - 6));
    } else {
      expected.push_back(0);
    }
  }

  encoderWithFiltersTest(std::move(filters), expected, 0);
}

} // namespace fbpcf::mpc_std_lib::oram
