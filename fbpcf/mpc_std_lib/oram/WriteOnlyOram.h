/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGenerator.h"
#include "fbpcf/mpc_std_lib/oram/IWriteOnlyOram.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T>
class WriteOnlyOram final : public IWriteOnlyOram<T> {
  using Role = typename IWriteOnlyOram<T>::Role;

 public:
  WriteOnlyOram(
      Role myRole,
      size_t size,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<ISinglePointArrayGenerator> generator,
      std::unique_ptr<IDifferenceCalculator<T>> calculator)
      : myRole_(myRole),
        size_(size),
        agent_(std::move(agent)),
        generator_(std::move(generator)),
        calculator_(std::move(calculator)),
        memory_(size_, T(0)) {}

  /**
   * @inherit doc
   */
  T publicRead(size_t publicIndex, Role receiver) const override;

  /**
   * @inherit doc
   */
  T secretRead(size_t publicIndex) const override;

  /**
   * @inherit doc
   */
  void obliviousAddBatch(
      const std::vector<std::vector<bool>>& indexShares,
      const std::vector<std::vector<bool>>& values) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    auto generatorTraffic = generator_->getTrafficStatistics();
    auto calculatorTraffic = calculator_->getTrafficStatistics();
    auto oramTraffic = agent_->getTrafficStatistics();
    return {
        generatorTraffic.first + calculatorTraffic.first + oramTraffic.first,
        generatorTraffic.second + calculatorTraffic.second +
            oramTraffic.second};
  }

 private:
  std::vector<std::vector<T>> generateMasks(
      const std::vector<std::vector<bool>>& indexShares,
      const std::vector<std::vector<bool>>& values) const;

  Role myRole_;
  size_t size_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<ISinglePointArrayGenerator> generator_;
  std::unique_ptr<IDifferenceCalculator<T>> calculator_;

  std::vector<T> memory_;
};

} // namespace fbpcf::mpc_std_lib::oram

#include "fbpcf/mpc_std_lib/oram/WriteOnlyOram_impl.h"
