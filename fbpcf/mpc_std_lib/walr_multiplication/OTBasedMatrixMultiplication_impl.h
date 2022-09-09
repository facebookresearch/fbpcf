/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include "fbpcf/mpc_std_lib/walr_multiplication/OTBasedMatrixMultiplication.h"

namespace fbpcf::mpc_std_lib::walr {

template <int schedulerId, typename FixedPointType>
std::vector<double> OTBasedMatrixMultiplication<schedulerId, FixedPointType>::
    matrixVectorMultiplication(
        const std::vector<std::vector<double>>& features,
        const frontend::Bit<true, schedulerId, true>& labels) const {
  if (!isFeatureOwner_) {
    throw std::runtime_error(
        "You are calling the wrong version of this function.");
  }

  // Each features[i] represents a column vector of length nFeatures.
  // There are nLabels such column vectors.
  size_t nLabels = features.size();
  if (nLabels != labels.getBatchSize()) {
    throw std::invalid_argument(
        "The input sizes are not compatible: "
        "The number of columns (features.size()) does not equal"
        "the number of labels.");
  }

  size_t nFeatures = features[0].size();
  if (nFeatures > std::numeric_limits<uint32_t>::max()) {
    throw std::runtime_error(
        "Due to the PRG implementation "
        "Currently we only support feature matrix "
        "with less than " +
        std::to_string(std::numeric_limits<uint32_t>::max()) + " rows.");
  }

  // Extracting label before running COTwRM to help the other party
  // extract its label share, which will be used as choice bit
  auto extractedLabels = labels.extractBit().getValue();

  // create COT with random message
  // For each column, we run the COTwRM once to generate two 128-bit random
  // messages k0 and k1, which will be used to generate the random additive
  // noise vectors.
  auto [sender0Messages, sender1Messages] = cotWRM_->send(nLabels);
  assert(sender0Messages.size() == nLabels);
  assert(sender1Messages.size() == nLabels);

  // Now we run the protocol
  std::vector<FixedPointType> totalNoise(nFeatures, 0);
  for (size_t i = 0; i < nLabels; i++) {
    std::vector<FixedPointType> feature =
        numberMapper_.mapToFixedPointType(features[i]);

    // Generate the additive noise vector using the random msg given by the OT
    auto prg0 = prgFactory_->create(sender0Messages[i]);
    auto key0long = prg0->getRandomUInt64(nFeatures);
    std::vector<FixedPointType> key0;
    std::transform(
        key0long.cbegin(),
        key0long.cend(),
        std::back_inserter(key0),
        [](uint64_t a) { return static_cast<FixedPointType>(a); });
    auto prg1 = prgFactory_->create(sender1Messages[i]);
    auto key1long = prg1->getRandomUInt64(nFeatures);
    std::vector<FixedPointType> key1;
    std::transform(
        key1long.cbegin(),
        key1long.cend(),
        std::back_inserter(key1),
        [](uint64_t a) { return static_cast<FixedPointType>(a); });

    // We pick a noise vector that makes the correction for 0-message always
    // equal 0. Then we only need to send the correction for 1-message
    std::vector<FixedPointType> noise;
    std::vector<FixedPointType> correction1;
    if (extractedLabels[i] == false) {
      // We are sending (r_i, r_i + M_i)
      // noise r_i = key0, so if the other party has m0, it can generate key0
      // directly without bother receiving a correction
      noise = key0;
      // correction1 = key1 - r_i - M_i
      correction1 = key1;
      std::transform(
          correction1.cbegin(),
          correction1.cend(),
          noise.cbegin(),
          correction1.begin(),
          std::minus<FixedPointType>());
      std::transform(
          correction1.cbegin(),
          correction1.cend(),
          feature.cbegin(),
          correction1.begin(),
          std::minus<FixedPointType>());
    } else {
      // We are sending (r_i + M_i, r_i)
      // Again, we make key0 - (r_i + M_i) = 0, so the other party can just
      // generate r_i + M_i if it has key0.
      noise = key0;
      std::transform(
          noise.cbegin(),
          noise.cend(),
          feature.cbegin(),
          noise.begin(),
          std::minus<FixedPointType>());

      // correction1 = key1 - r_i
      correction1 = key1;
      std::transform(
          correction1.cbegin(),
          correction1.cend(),
          noise.cbegin(),
          correction1.begin(),
          std::minus<FixedPointType>());
    }
    // record the added noise vector
    std::transform(
        totalNoise.cbegin(),
        totalNoise.cend(),
        noise.cbegin(),
        totalNoise.begin(),
        std::plus<FixedPointType>());

    agent_->sendT<FixedPointType>(correction1);
  }

  // receive the sum (with dp noise) from the other party
  auto maskedResult = agent_->receiveT<FixedPointType>(nFeatures);
  // remove the additive noise
  std::transform(
      maskedResult.cbegin(),
      maskedResult.cend(),
      totalNoise.cbegin(),
      maskedResult.begin(),
      std::minus<FixedPointType>());

  std::vector<double> rst = numberMapper_.mapToSignedDouble(maskedResult);
  return rst;
}

template <int schedulerId, typename FixedPointType>
void OTBasedMatrixMultiplication<schedulerId, FixedPointType>::
    matrixVectorMultiplication(
        const frontend::Bit<true, schedulerId, true>& labels,
        const std::vector<double>& dpNoise) const {
  if (isFeatureOwner_) {
    throw std::runtime_error(
        "You are calling the wrong version of this function.");
  }

  size_t nFeatures = dpNoise.size();
  size_t nLabels = labels.getBatchSize();

  // Run the COT-with-Random-Message protocol
  auto choice = labels.extractBit().getValue();
  auto receiverMessages = cotWRM_->receive(choice);
  assert(receiverMessages.size() == nLabels);

  // Run the matrix multiplication protocol
  std::vector<FixedPointType> rst(nFeatures, 0);
  for (size_t i = 0; i < nLabels; ++i) {
    auto correction1 = agent_->receiveT<FixedPointType>(nFeatures);

    // correct the received feature vector
    auto prg = prgFactory_->create(receiverMessages.at(i));
    auto keyLong = prg->getRandomUInt64(nFeatures);
    std::vector<FixedPointType> key;
    std::transform(
        keyLong.cbegin(),
        keyLong.cend(),
        std::back_inserter(key),
        [](uint64_t a) { return static_cast<FixedPointType>(a); });
    // When choice[i] == false, we have key0, and the corrected msg is exactly
    // key0. Otherwise we have key1, and we obtain the corrected msg as key1 -
    // correction1
    if (choice[i] == true) {
      std::transform(
          key.cbegin(),
          key.cend(),
          correction1.cbegin(),
          key.begin(),
          std::minus<FixedPointType>());
    }

    // add the corrected message
    std::transform(
        rst.cbegin(),
        rst.cend(),
        key.cbegin(),
        rst.begin(),
        std::plus<FixedPointType>());
  }
  // impose the DP noise
  auto convertedDpNoise = numberMapper_.mapToFixedPointType(dpNoise);
  std::transform(
      rst.cbegin(),
      rst.cend(),
      convertedDpNoise.cbegin(),
      rst.begin(),
      std::plus<FixedPointType>());

  // share the final result with the other party
  agent_->sendT<FixedPointType>(rst);
}

} // namespace fbpcf::mpc_std_lib::walr
