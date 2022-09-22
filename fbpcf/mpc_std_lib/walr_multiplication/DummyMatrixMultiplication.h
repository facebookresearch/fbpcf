/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/engine/util/util.h>
#include <algorithm>
#include <stdexcept>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"

namespace fbpcf::mpc_std_lib::walr::insecure {

/**
 * An insecure multiplication implementation. It should not be used in
 * production.
 */
template <int schedulerId>
class DummyMatrixMultiplication final
    : public IWalrMatrixMultiplication<schedulerId> {
 public:
  explicit DummyMatrixMultiplication(
      int myId,
      int partnerId,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent)
      : myId_(myId), partnerId_(partnerId), agent_(std::move(agent)) {}

  std::pair<uint64_t, uint64_t> getNonEngineTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 protected:
  std::vector<double> matrixVectorMultiplicationImpl(
      const std::vector<std::vector<double>>& features,
      const frontend::Bit<true, schedulerId, true>& labels) const override {
    // Each features[i] represents a column vector of the feature matrix.
    // There are `nLabels` such column vectors.
    size_t nLabels = labels.getBatchSize();
    if (nLabels != features.size()) {
      throw std::invalid_argument(
          "The input sizes are not compatible: "
          "The number of columns (features.size()) does not equal"
          "the number of labels.");
    }

    size_t nFeatures = features[0].size();
    std::vector<double> rst(nFeatures, 0);
    auto revealedValues = labels.openToParty(myId_).getValue();
    for (size_t i = 0; i < nLabels; ++i) {
      if (features[i].size() != nFeatures) {
        throw std::invalid_argument(
            "Columns of the feature matrix have different sizes.");
      }

      if (revealedValues[i]) {
        std::transform( // add the column vector to rst
            rst.cbegin(),
            rst.cend(),
            features[i].cbegin(),
            rst.begin(),
            std::plus<double>());
      }
    }

    // receive the DP noise from the label owner
    std::vector<double> dpNoise = agent_->receiveT<double>(nFeatures);
    std::transform( // add the DP noise to rst
        rst.cbegin(),
        rst.cend(),
        dpNoise.cbegin(),
        rst.begin(),
        std::plus<double>());
    return rst;
  }

  void matrixVectorMultiplicationImpl(
      const frontend::Bit<true, schedulerId, true>& labels,
      const std::vector<double>& dpNoise) const override {
    labels.openToParty(partnerId_).getValue();
    agent_->sendT<double>(dpNoise);
  }

 private:
  int myId_;
  int partnerId_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
};
} // namespace fbpcf::mpc_std_lib::walr::insecure
