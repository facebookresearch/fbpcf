/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <smmintrin.h>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBaseObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IknpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {
void testRandomCorrelatedObliviousTransfer(
    std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory0,
    std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory1) {
  communication::InMemoryPartyCommunicationAgentHost host;

  auto agent0 = host.getAgent(0);
  auto agent1 = host.getAgent(1);

  __m128i delta = _mm_set_epi32(0xFFFFFFFF, 0, 0, 1);

  int64_t size = 16384;

  auto senderTask =
      [delta, size](
          std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto ot = factory->create(delta, std::move(agent));
        std::vector<__m128i> rst;
        for (int i = 0; i < 128; i++) {
          auto tmp = ot->rcot(size);
          EXPECT_EQ(tmp.size(), size);
          rst.insert(rst.end(), tmp.begin(), tmp.end());
        }
        return rst;
      };

  auto receiverTask =
      [size](
          std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto ot = factory->create(std::move(agent));
        std::vector<__m128i> rst;
        for (int i = 0; i < 128; i++) {
          auto tmp = ot->rcot(size);
          EXPECT_EQ(tmp.size(), size);
          rst.insert(rst.end(), tmp.begin(), tmp.end());
        }
        return rst;
      };

  auto f0 = std::async(senderTask, std::move(factory0), std::move(agent0));
  auto f1 = std::async(receiverTask, std::move(factory1), std::move(agent1));

  auto resultSend = f0.get();
  auto resultReceive = f1.get();

  for (int i = 0; i < resultSend.size(); i++) {
    EXPECT_TRUE(
        // 0 message, lsbs are both 0
        compareM128i(resultSend[i], resultReceive[i]) ||
        // 1 message, lsbs are both 1
        compareM128i(_mm_xor_si128(resultSend[i], delta), resultReceive[i]));
  }
}

TEST(RandomCorrelatedObliviousTransferTest, testDummyRcot) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<
          insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
      std::make_unique<
          insecure::DummyRandomCorrelatedObliviousTransferFactory>());
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testExtenderBasedRcotWithDummyExtender) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::insecure::DummyRcotExtenderFactory>(),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::insecure::DummyRcotExtenderFactory>(),
          1024,
          128,
          8));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testExtenderBasedRcotWithFerretExtenderPoweredByDummyMpcotAndDummyMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<
                  ferret::insecure::DummyMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<
                  ferret::insecure::DummyMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testExtenderBasedRcotWithFerretExtenderPoweredByDummyMpcotAnd10LocalLinearMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testExtenderBasedRcotWithFerretExtenderPoweredByMpcotWithDummySpcotAnd10LocalLinearMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<
                      ferret::insecure::DummySinglePointCotFactory>(
                      std::make_unique<util::AesPrgFactory>(1024)))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<
                      ferret::insecure::DummySinglePointCotFactory>(
                      std::make_unique<util::AesPrgFactory>(1024)))),
          1024,
          128,
          8));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testExtenderBasedRcotWithFerretExtenderPoweredByMpcotWithRealSpcotAnd10LocalLinearMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight));
}

TEST(RandomCorrelatedObliviousTransferTest, testEmpRcot) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<util::AesPrgFactory>(1024)),
      std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<util::AesPrgFactory>(1024)));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testBootstrappedExtenderBasedRcotWithFerretExtenderPoweredByMpcotWithRealSpcotAnd10LocalLinearMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<util::AesPrgFactory>(1024)),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<util::AesPrgFactory>(1024)),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight));
}

bool extractIJ(const std::vector<__m128i>& matrixes, int index, int i, int j) {
  assert(matrixes.size() >= (index + 1) * 128);
  assert(i < 128);
  assert(j < 128);
  auto tmp0 = matrixes.at(index * 128 + i);
  if (j < 64) {
    uint64_t tmp1 = _mm_extract_epi64(tmp0, 0);
    return (tmp1 >> j) & 1;
  } else {
    uint64_t tmp1 = _mm_extract_epi64(tmp0, 1);
    return (tmp1 >> (j - 64)) & 1;
  }
}

class IKNPMatrixTransposeTestHelper final
    : IknpShRandomCorrelatedObliviousTransfer {
  FRIEND_TEST(IKNPRandomCorrelatedObliviousTransferTest, testMatrixTranspose);
};

TEST(IKNPRandomCorrelatedObliviousTransferTest, testMatrixTranspose) {
  int size = 128;
  std::vector<__m128i> testData(size * 128);
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
  for (auto& item : testData) {
    item = _mm_set_epi64x(dist(e), dist(e));
  }
  auto dst0 = IKNPMatrixTransposeTestHelper::matrixTranspose(testData);

  for (size_t index = 0; index < size; index++) {
    for (int i = 0; i < 128; i++) {
      for (int j = 0; j < 128; j++) {
        EXPECT_EQ(
            extractIJ(testData, index, i, j), extractIJ(dst0, index, j, i));
      }
    }
  }

  auto dst1 = IKNPMatrixTransposeTestHelper::matrixTranspose(dst0);

  for (size_t i = 0; i < testData.size(); i++) {
    EXPECT_TRUE(compareM128i(dst1.at(i), testData.at(i)));
  }
}

TEST(
    IKNPRandomCorrelatedObliviousTransferTest,
    testIKNPRandomCorrelatedObliviousTransferWithDummyBaseOt) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<insecure::DummyBaseObliviousTransferFactory>()),
      std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<insecure::DummyBaseObliviousTransferFactory>()));
}

TEST(
    IKNPRandomCorrelatedObliviousTransferTest,
    testIKNPRandomCorrelatedObliviousTransferWithNpBaseOt) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<NpBaseObliviousTransferFactory>()),
      std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<NpBaseObliviousTransferFactory>()));
}

TEST(
    RandomCorrelatedObliviousTransferTest,
    testIKNPBootstrappedExtenderBasedRcotWithFerretExtenderPoweredByMpcotWithRealSpcotAnd10LocalLinearMatrixMultipler) {
  testRandomCorrelatedObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<NpBaseObliviousTransferFactory>()),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<IknpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<NpBaseObliviousTransferFactory>()),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight));
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
