/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/engine/util/AesPrgFactory.h"

#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/OTBasedMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessageFactory.h"

#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::walr {

std::vector<std::vector<double>> generateRandomFeatures(
    size_t nFeatures,
    size_t nLabels,
    double a = 0.0,
    double b = 1.0) {
  std::vector<std::vector<double>> features;
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_real_distribution<double> dist(a, b);
  for (int i = 0; i < nLabels; ++i) {
    std::vector<double> column(nFeatures);
    std::generate(
        column.begin(), column.end(), [&dist, &e]() { return dist(e); });
    features.push_back(column);
  }
  return features;
}

std::vector<bool> generateRandomLabels(size_t nLabels, double p = 0.5) {
  std::vector<bool> labels(nLabels);
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::bernoulli_distribution dist(p);
  std::generate(
      labels.begin(), labels.end(), [&dist, &e]() { return dist(e); });
  return labels;
}

std::vector<double>
generateRandomNoise(size_t n, double a = 0, double b = 1.0) {
  std::vector<double> noise(n);
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_real_distribution dist(a, b);
  std::generate(noise.begin(), noise.end(), [&dist, &e]() { return dist(e); });
  return noise;
}

// This should be invoked only after corresponding schedulers are created
// The secret label is owned by party.
template <int featureOwnerSchedulerId, int labelOwnerSchedulerId>
std::pair<
    frontend::Bit<true, featureOwnerSchedulerId, true>,
    frontend::Bit<true, labelOwnerSchedulerId, true>>
generateSecretLabelShares(
    const std::vector<bool>& labelValues,
    int labelOwnerPartyId) {
  frontend::Bit<true, featureOwnerSchedulerId, true> featureOwnerShare(
      std::vector<bool>(labelValues.size(), true), labelOwnerPartyId);
  frontend::Bit<true, labelOwnerSchedulerId, true> labelOwnerShare(
      labelValues, labelOwnerPartyId);
  return {featureOwnerShare, labelOwnerShare};
}

std::vector<double> plaintextMatrixVectorMultiplication(
    const std::vector<std::vector<double>>& features,
    const std::vector<bool>& labels) {
  std::vector<double> rst(features.at(0).size());
  for (int i = 0; i < labels.size(); ++i) {
    if (labels[i]) {
      std::transform(
          rst.cbegin(),
          rst.cend(),
          features[i].cbegin(),
          rst.begin(),
          std::plus<double>());
    }
  }
  return rst;
}

// helper function for comparing two double vectors
void testVectorAlmostEq(
    const std::vector<double>& vec1,
    const std::vector<double>& vec2,
    double absError) {
  ASSERT_EQ(vec1.size(), vec2.size());
  for (size_t i = 0; i < vec1.size(); ++i) {
    // Test if the absolute error is within tolerance
    EXPECT_NEAR(vec1[i], vec2[i], absError) << "at position: " << i;
  }
}

// template test for matrix-vector multiplication
template <
    int featureOwnerSchedulerId,
    int labelOwnerSchedulerId,
    int featureOwnerId,
    int labelOwnerId>
void matrixVectorMultiplicationTestHelper(
    std::unique_ptr<IWalrMatrixMultiplicationFactory<featureOwnerSchedulerId>>
        featureOwnerFactory,
    std::unique_ptr<IWalrMatrixMultiplicationFactory<labelOwnerSchedulerId>>
        labelOwnerFactory,
    std::function<void(size_t, size_t)> featureOwnerMetricTester,
    std::function<void(size_t, size_t)> labelOwnerMetricTester,
    const std::vector<std::vector<double>>& testFeatures,
    const std::vector<bool>& testLabelValues,
    const std::vector<double>& testDpNoise,
    const std::vector<double>& expectedOutput,
    double tolerance = 1e-7) {
  // setup mpc engine and schedulers
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackendWithLazyScheduler<
      featureOwnerSchedulerId,
      labelOwnerSchedulerId>(
      *agentFactories[featureOwnerId], *agentFactories[labelOwnerId]);

  frontend::Bit<true, featureOwnerSchedulerId, true> testLabelsShare0;
  frontend::Bit<true, labelOwnerSchedulerId, true> testLabelsShare1;
  std::tie(testLabelsShare0, testLabelsShare1) =
      generateSecretLabelShares<featureOwnerSchedulerId, labelOwnerSchedulerId>(
          testLabelValues, labelOwnerId);

  auto task0 = [&testFeatures, &testLabelsShare0](
                   std::unique_ptr<IWalrMatrixMultiplicationFactory<
                       featureOwnerSchedulerId>> partyFactory) {
    return partyFactory->create()->matrixVectorMultiplication(
        testFeatures, testLabelsShare0);
  };
  auto task1 = [&testLabelsShare1](
                   std::unique_ptr<IWalrMatrixMultiplicationFactory<
                       labelOwnerSchedulerId>> partyFactory,
                   const std::vector<double>& dpNoise) {
    partyFactory->create()->matrixVectorMultiplication(
        testLabelsShare1, dpNoise);
  };

  auto future0 = std::async(task0, std::move(featureOwnerFactory));
  auto future1 = std::async(task1, std::move(labelOwnerFactory), testDpNoise);
  auto rst = future0.get();
  future1.get();

  testVectorAlmostEq(rst, expectedOutput, tolerance);
  auto nLabels = testFeatures.size();
  auto nFeatures = testFeatures.at(0).size();
  featureOwnerMetricTester(nFeatures, nLabels);
  labelOwnerMetricTester(nFeatures, nLabels);
}

template <
    int featureOwnerSchedulerId,
    int labelOwnerSchedulerId,
    int featureOwnerId,
    int labelOwnerId>
void matrixVectorMultiplicationNoNoiseTest(
    std::unique_ptr<IWalrMatrixMultiplicationFactory<featureOwnerSchedulerId>>
        featureOwnerFactory,
    std::unique_ptr<IWalrMatrixMultiplicationFactory<labelOwnerSchedulerId>>
        labelOwnerFactory,
    std::function<void(size_t, size_t)> featureOwnerMetricTester,
    std::function<void(size_t, size_t)> labelOwnerMetricTester,
    double tolerance = 1e-7) {
  // generate test data
  size_t nFeatures = 150;
  size_t nLabels = 200;
  // Random features and labels
  auto testFeatures = generateRandomFeatures(nFeatures, nLabels, -2.0, 4.0);
  auto testLabelValues = generateRandomLabels(nLabels);

  auto expectedOutput =
      plaintextMatrixVectorMultiplication(testFeatures, testLabelValues);

  matrixVectorMultiplicationTestHelper<
      featureOwnerSchedulerId,
      labelOwnerSchedulerId,
      featureOwnerId,
      labelOwnerId>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      featureOwnerMetricTester,
      labelOwnerMetricTester,
      testFeatures,
      testLabelValues,
      std::vector<double>(nFeatures, 0.0), // no DP noise
      expectedOutput,
      tolerance);
}

// (2) Test with uniform random noise
template <
    int featureOwnerSchedulerId,
    int labelOwnerSchedulerId,
    int featureOwnerId,
    int labelOwnerId>
void matrixVectorMultiplicationUniformNoiseTest(
    std::unique_ptr<IWalrMatrixMultiplicationFactory<featureOwnerSchedulerId>>
        featureOwnerFactory,
    std::unique_ptr<IWalrMatrixMultiplicationFactory<labelOwnerSchedulerId>>
        labelOwnerFactory,
    std::function<void(size_t, size_t)> featureOwnerMetricTester,
    std::function<void(size_t, size_t)> labelOwnerMetricTester,
    double tolerance = 1e-7) {
  // generate test data
  size_t nFeatures = 200;
  size_t nLabels = 150;
  // Random features and labels
  auto testFeatures = generateRandomFeatures(nFeatures, nLabels, -2.0, 4.0);
  auto testLabelValues = generateRandomLabels(nLabels);

  auto expectedOutput =
      plaintextMatrixVectorMultiplication(testFeatures, testLabelValues);
  auto magnitude = *std::max_element(
      expectedOutput.cbegin(), expectedOutput.cend(), [](double a, double b) {
        return (std::abs(a) < std::abs(b));
      });
  magnitude = std::abs(magnitude);
  auto uniformNoise = generateRandomNoise(nFeatures, -2 * magnitude, magnitude);

  // Add the noise to the expectedOutput
  std::transform(
      expectedOutput.cbegin(),
      expectedOutput.cend(),
      uniformNoise.cbegin(),
      expectedOutput.begin(),
      std::plus<double>());

  matrixVectorMultiplicationTestHelper<
      featureOwnerSchedulerId,
      labelOwnerSchedulerId,
      featureOwnerId,
      labelOwnerId>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      featureOwnerMetricTester,
      labelOwnerMetricTester,
      testFeatures,
      testLabelValues,
      uniformNoise,
      expectedOutput,
      tolerance);
}

auto metricTesterForDummyMM(
    std::shared_ptr<fbpcf::util::MetricCollector> featureOwnerCollector,
    std::shared_ptr<fbpcf::util::MetricCollector> labelOwnerCollector) {
  std::string name = "dummy_matrix_multiplication";

  auto tester0 = [=](size_t nFeatures, [[maybe_unused]] size_t nLabels) {
    auto metrics = featureOwnerCollector->collectMetrics();
    auto collectorPrefix = featureOwnerCollector->getPrefix();
    ASSERT_EQ(metrics[collectorPrefix + "." + name]["features_sent"], 0);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name]["features_received"], nFeatures);
  };

  auto tester1 = [=](size_t nFeatures, [[maybe_unused]] size_t nLabels) {
    auto metrics = labelOwnerCollector->collectMetrics();
    auto collectorPrefix = labelOwnerCollector->getPrefix();
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name]["features_sent"], nFeatures);
    ASSERT_EQ(metrics[collectorPrefix + "." + name]["features_received"], 0);
  };

  return std::make_pair(tester0, tester1);
}

auto metricTesterForOTBasedMM(
    std::shared_ptr<fbpcf::util::MetricCollector> featureOwnerCollector,
    std::shared_ptr<fbpcf::util::MetricCollector> labelOwnerCollector) {
  // recorder names
  std::string name0 = "ot_based_matrix_multiplication_feature_owner";
  std::string name1 = "ot_based_matrix_multiplication_label_owner";

  auto tester0 = [=](size_t nFeatures, size_t nLabels) {
    auto metrics = featureOwnerCollector->collectMetrics();
    auto collectorPrefix = featureOwnerCollector->getPrefix();
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name0]["ot_messages_used"], nLabels);
    ASSERT_EQ(metrics[collectorPrefix + "." + name0]["columns_sent"], nLabels);
    ASSERT_EQ(metrics[collectorPrefix + "." + name0]["columns_received"], 1);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name0]["features_sent"],
        nFeatures * nLabels);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name0]["features_received"], nFeatures);
  };

  auto tester1 = [=](size_t nFeatures, size_t nLabels) {
    auto metrics = labelOwnerCollector->collectMetrics();
    auto collectorPrefix = labelOwnerCollector->getPrefix();
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name1]["ot_messages_used"], nLabels);
    ASSERT_EQ(metrics[collectorPrefix + "." + name1]["columns_sent"], 1);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name1]["columns_received"], nLabels);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name1]["features_sent"], nFeatures);
    ASSERT_EQ(
        metrics[collectorPrefix + "." + name1]["features_received"],
        nFeatures * nLabels);
  };

  return std::make_pair(tester0, tester1);
}

TEST(matrixVectorMultiplicationNoNoiseTest, testDummyMatrixMultiplication) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "dummy_matrix_multiplication_no_noise_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "dummy_matrix_multiplication_no_noise_test_party_1");
  auto [tester0, tester1] = metricTesterForDummyMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<0>>(
          0, 1, *agentFactories[0], collector0);
  auto labelOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<1>>(
          1, 0, *agentFactories[1], collector1);

  matrixVectorMultiplicationNoNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      1e-7);
}

TEST(
    matrixVectorMultiplicationUniformNoiseTest,
    testDummyMatrixMultiplication) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "dummy_matrix_multiplication_uniform_noise_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "dummy_matrix_multiplication_uniform_noise_test_party_1");
  auto [tester0, tester1] = metricTesterForDummyMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<0>>(
          0, 1, *agentFactories[0], collector0);
  auto labelOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<1>>(
          1, 0, *agentFactories[1], collector1);

  matrixVectorMultiplicationUniformNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      1e-7);
}

TEST(
    matrixVectorMultiplicationNoNoiseTest,
    testOTBasedMatrixMultiplicationWithDummyRCOT) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  constexpr double tolerance = 1e-7;

  auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
  auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

  auto rcotFactory0 =
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>();
  auto rcotFactory1 =
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>();

  auto cotWRMFactory0 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory0));
  auto cotWRMFactory1 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory1));

  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_no_noise_with_dummy_rcot_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_no_noise_with_dummy_rcot_test_party_1");
  auto [tester0, tester1] = metricTesterForOTBasedMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<0, uint64_t>>(
          0,
          1,
          true,
          divisor,
          *agentFactories[0],
          std::move(prgFactory0),
          std::move(cotWRMFactory0),
          collector0);
  auto labelOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<1, uint64_t>>(
          1,
          0,
          false,
          divisor,
          *agentFactories[1],
          std::move(prgFactory1),
          std::move(cotWRMFactory1),
          collector1);

  matrixVectorMultiplicationNoNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      tolerance);
}

TEST(
    matrixVectorMultiplicationUniformNoiseTest,
    testOTBasedMatrixMultiplicationWithDummyRCOT) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  constexpr double tolerance = 1e-7;

  auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
  auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

  auto rcotFactory0 =
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>();
  auto rcotFactory1 =
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>();

  auto cotWRMFactory0 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory0));
  auto cotWRMFactory1 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory1));

  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_uniform_noise_with_dummy_rcot_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_uniform_noise_with_dummy_rcot_test_party_1");
  auto [tester0, tester1] = metricTesterForOTBasedMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<0, uint64_t>>(
          0,
          1,
          true,
          divisor,
          *agentFactories[0],
          std::move(prgFactory0),
          std::move(cotWRMFactory0),
          collector0);
  auto labelOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<1, uint64_t>>(
          1,
          0,
          false,
          divisor,
          *agentFactories[1],
          std::move(prgFactory1),
          std::move(cotWRMFactory1),
          collector1);

  matrixVectorMultiplicationUniformNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      tolerance);
}

TEST(
    matrixVectorMultiplicationNoNoiseTest,
    testOTBasedMatrixMultiplicationWithFERRET) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  constexpr double tolerance = 1e-7;

  auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
  auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

  auto rcotFactory0 = std::make_unique<
      engine::tuple_generator::oblivious_transfer::
          ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<engine::tuple_generator::oblivious_transfer::
                           EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<engine::util::AesPrgFactory>(1024)),
      std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                           RcotExtenderFactory>(
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               TenLocalLinearMatrixMultiplierFactory>(),
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               RegularErrorMultiPointCotFactory>(
              std::make_unique<engine::tuple_generator::oblivious_transfer::
                                   ferret::SinglePointCotFactory>())),
      engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
      engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
      engine::tuple_generator::oblivious_transfer::ferret::kWeight);
  auto rcotFactory1 = std::make_unique<
      engine::tuple_generator::oblivious_transfer::
          ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<engine::tuple_generator::oblivious_transfer::
                           EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<engine::util::AesPrgFactory>(1024)),
      std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                           RcotExtenderFactory>(
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               TenLocalLinearMatrixMultiplierFactory>(),
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               RegularErrorMultiPointCotFactory>(
              std::make_unique<engine::tuple_generator::oblivious_transfer::
                                   ferret::SinglePointCotFactory>())),
      engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
      engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
      engine::tuple_generator::oblivious_transfer::ferret::kWeight);

  auto cotWRMFactory0 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory0));
  auto cotWRMFactory1 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory1));

  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_no_noise_with_ferret_rcot_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_no_noise_with_ferret_rcot_test_party_1");
  auto [tester0, tester1] = metricTesterForOTBasedMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<0, uint64_t>>(
          0,
          1,
          true,
          divisor,
          *agentFactories[0],
          std::move(prgFactory0),
          std::move(cotWRMFactory0),
          collector0);
  auto labelOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<1, uint64_t>>(
          1,
          0,
          false,
          divisor,
          *agentFactories[1],
          std::move(prgFactory1),
          std::move(cotWRMFactory1),
          collector1);

  matrixVectorMultiplicationUniformNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      tolerance);
}

TEST(
    matrixVectorMultiplicationUniformNoiseTest,
    testOTBasedMatrixMultiplicationWithFERRET) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  constexpr double tolerance = 1e-7;

  auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
  auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

  auto rcotFactory0 = std::make_unique<
      engine::tuple_generator::oblivious_transfer::
          ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<engine::tuple_generator::oblivious_transfer::
                           EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<engine::util::AesPrgFactory>(1024)),
      std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                           RcotExtenderFactory>(
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               TenLocalLinearMatrixMultiplierFactory>(),
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               RegularErrorMultiPointCotFactory>(
              std::make_unique<engine::tuple_generator::oblivious_transfer::
                                   ferret::SinglePointCotFactory>())),
      engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
      engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
      engine::tuple_generator::oblivious_transfer::ferret::kWeight);
  auto rcotFactory1 = std::make_unique<
      engine::tuple_generator::oblivious_transfer::
          ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<engine::tuple_generator::oblivious_transfer::
                           EmpShRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<engine::util::AesPrgFactory>(1024)),
      std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                           RcotExtenderFactory>(
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               TenLocalLinearMatrixMultiplierFactory>(),
          std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                               RegularErrorMultiPointCotFactory>(
              std::make_unique<engine::tuple_generator::oblivious_transfer::
                                   ferret::SinglePointCotFactory>())),
      engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
      engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
      engine::tuple_generator::oblivious_transfer::ferret::kWeight);

  auto cotWRMFactory0 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory0));
  auto cotWRMFactory1 = std::make_unique<util::COTWithRandomMessageFactory>(
      std::move(rcotFactory1));

  auto collector0 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_uniform_noise_with_ferret_rcot_test_party_0");
  auto collector1 = std::make_shared<fbpcf::util::MetricCollector>(
      "ot_based_matrix_multiplication_uniform_noise_with_ferret_rcot_test_party_1");
  auto [tester0, tester1] = metricTesterForOTBasedMM(collector0, collector1);

  auto featureOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<0, uint64_t>>(
          0,
          1,
          true,
          divisor,
          *agentFactories[0],
          std::move(prgFactory0),
          std::move(cotWRMFactory0),
          collector0);
  auto labelOwnerFactory =
      std::make_unique<OTBasedMatrixMultiplicationFactory<1, uint64_t>>(
          1,
          0,
          false,
          divisor,
          *agentFactories[1],
          std::move(prgFactory1),
          std::move(cotWRMFactory1),
          collector1);

  matrixVectorMultiplicationUniformNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory),
      std::move(labelOwnerFactory),
      tester0,
      tester1,
      tolerance);
}
} // namespace fbpcf::mpc_std_lib::walr
