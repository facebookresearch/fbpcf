/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IDifferenceCalculatorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/ISinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IWriteOnlyOramFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/WriteOnlyOram.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

template <typename T>
class WriteOnlyOramFactory final : public IWriteOnlyOramFactory<T> {
 public:
  WriteOnlyOramFactory(
      typename IWriteOnlyOram<T>::Role myRole,
      int32_t peerId,
      engine::communication::IPartyCommunicationAgentFactory& factory,
      std::unique_ptr<ISinglePointArrayGeneratorFactory>
          singlePointArrayFactory,
      std::unique_ptr<IDifferenceCalculatorFactory<T>>
          differenceCalculatorFactory)
      : myRole_(myRole),
        peerId_(peerId),
        factory_(factory),
        singlePointArrayFactory_(std::move(singlePointArrayFactory)),
        differenceCalculatorFactory_(std::move(differenceCalculatorFactory)) {}

  std::unique_ptr<IWriteOnlyOram<T>> create(size_t size) override {
    return std::make_unique<WriteOnlyOram<T>>(
        myRole_,
        size,
        factory_.create(peerId_),
        singlePointArrayFactory_->create(),
        differenceCalculatorFactory_->create());
  }

  /**
   * @inherit doc
   */
  uint32_t getMaxBatchSize(size_t size, uint8_t concurrency) override {
    // Split ORAM input into batches to prevent OOM errors.
    assert(concurrency <= maxConcurrency);
    uint8_t oramWidth = std::ceil(std::log2(size));
    uint32_t maxBatchSize = static_cast<uint32_t>(std::pow(
        2,
        intercept0 + intercept1 * concurrency +
            intercept2 * std::pow(2, concurrency) + slope * oramWidth));
    return maxBatchSize;
  }

 private:
  typename IWriteOnlyOram<T>::Role myRole_;
  int32_t peerId_;
  engine::communication::IPartyCommunicationAgentFactory& factory_;
  std::unique_ptr<ISinglePointArrayGeneratorFactory> singlePointArrayFactory_;
  std::unique_ptr<IDifferenceCalculatorFactory<T>> differenceCalculatorFactory_;

  // The formula for the batch size is only valid with concurrency at most 4,
  // where we choose 4 because we use an AWS container with 4 vCPUs.
  const int maxConcurrency = 4;
  const double intercept0 = 25.3;
  const double intercept1 = -1.2;
  const double intercept2 = 0.1;
  const double slope = -0.757;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
