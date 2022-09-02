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
#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplicationFactory.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::walr {

std::vector<std::vector<double>> generateRandomFeatures(
    size_t nFeatures,
    size_t nLabels) {
  std::vector<std::vector<double>> features;
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_real_distribution<double> dist(0, 1.0);
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
    const std::vector<std::vector<double>>& testFeatures,
    const std::vector<bool>& testLabelValues,
    const std::vector<double>& testDpNoise,
    const std::vector<double>& expectedOutput,
    double tolerance = 1e-7) {
  // setup mpc engine and schedulers
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<featureOwnerSchedulerId, labelOwnerSchedulerId>(
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
    double tolerance = 1e-7) {
  // generate test data
  size_t nFeatures = 150;
  size_t nLabels = 200;
  // Random features and labels
  auto testFeatures = generateRandomFeatures(nFeatures, nLabels);
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
    double tolerance = 1e-7) {
  // generate test data
  size_t nFeatures = 200;
  size_t nLabels = 150;
  // Random features and labels
  auto testFeatures = generateRandomFeatures(nFeatures, nLabels);
  auto testLabelValues = generateRandomLabels(nLabels);

  auto expectedOutput =
      plaintextMatrixVectorMultiplication(testFeatures, testLabelValues);
  auto uniformNoise = generateRandomNoise(nFeatures, -5, 5);

  // Cap the noise so that it does not make the expectedOutput negative.
  // Because we are not supposed to handle negative numbers.
  std::transform(
      uniformNoise.cbegin(),
      uniformNoise.cend(),
      expectedOutput.cbegin(),
      uniformNoise.begin(),
      [tolerance](double noise, double output) {
        if (noise + output < 0) {
          return -output + std::min(tolerance, 1e-12);
        } else {
          return noise;
        }
      });

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
      testFeatures,
      testLabelValues,
      uniformNoise,
      expectedOutput,
      tolerance);
}

TEST(matrixVectorMultiplicationNoNoiseTest, testDummyMatrixMultiplication) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  auto featureOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<0>>(
          0, 1, *agentFactories[0]);
  auto labelOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<1>>(
          1, 0, *agentFactories[1]);

  matrixVectorMultiplicationNoNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory), std::move(labelOwnerFactory), 1e-7);
}

TEST(
    matrixVectorMultiplicationUniformNoiseTest,
    testDummyMatrixMultiplication) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  // feature owner is party 0 and uses scheduler 0
  // label owner is party 1 and uses scheduler 1
  auto featureOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<0>>(
          0, 1, *agentFactories[0]);
  auto labelOwnerFactory =
      std::make_unique<insecure::DummyMatrixMultiplicationFactory<1>>(
          1, 0, *agentFactories[1]);

  matrixVectorMultiplicationUniformNoiseTest<0, 1, 0, 1>(
      std::move(featureOwnerFactory), std::move(labelOwnerFactory), 1e-7);
}
} // namespace fbpcf::mpc_std_lib::walr
