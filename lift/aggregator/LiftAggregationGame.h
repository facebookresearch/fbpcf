/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <memory>
#include <vector>

#include <folly/logging/xlog.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../common/GroupedLiftMetrics.h"
#include "../common/PrivateData.h"
#include "MetricsMapper.h"

namespace private_lift {
template <class IOChannel>
class LiftAggregationGame : public pcf::EmpGame<
                                IOChannel,
                                std::vector<GroupedLiftMetrics>,
                                GroupedLiftMetrics> {
 public:
  LiftAggregationGame(
      std::unique_ptr<IOChannel> ioChannel,
      pcf::Party party,
      pcf::Visibility visibility = pcf::Visibility::Public)
      : pcf::EmpGame<
            IOChannel,
            std::vector<GroupedLiftMetrics>,
            GroupedLiftMetrics>(std::move(ioChannel), party),
        visibility_{visibility} {}

  GroupedLiftMetrics play(
      const std::vector<GroupedLiftMetrics>& inputData) override {
    XLOG(INFO) << "Decoding metrics...";
    // XOR all metrics, return std::vector<std::vector<emp::Integer>>
    auto vv =
        pcf::functional::map<GroupedLiftMetrics, std::vector<emp::Integer>>(
            inputData, [](const auto& metrics) {
              auto empVector = mapGroupedLiftMetricsToEmpVector(metrics);
              return empVector.map(
                  [](const auto& x, const auto& y) { return x ^ y; });
            });

    // We need to ensure all vectors are of equal length
    auto maxSize = vv.at(0).size();
    for (const auto &vec : vv) {
      maxSize = std::max(maxSize, vec.size());
    }
    for (std::size_t i = 0; i < vv.size(); ++i) {
      if (vv.at(i).size() != maxSize) {
        XLOG(INFO) << "Padding next vector with zeroes to match length (vec["
                   << i << "].size() = " << vv.at(i).size()
                   << ", maxSize = " << maxSize << ")";
        while (vv.at(i).size() != maxSize) {
          vv.at(i).emplace_back(INT_SIZE, 0, emp::PUBLIC);
        }
      }
    }

    XLOG(INFO) << "Aggregating metrics...";
    // Aggregate all metrics, returns std::vector<emp::Integer>
    auto v = pcf::functional::reduce<std::vector<emp::Integer>>(
        vv, pcf::vector::Add<emp::Integer>);

    XLOG(INFO) << "Revealing metrics...";
    XLOGF(DBG, "Visibility: {}", this->visibility_);
    // Reveal aggregated metrics
    auto revealed = pcf::functional::map<emp::Integer, int64_t>(
        v, [visibility = visibility_](const emp::Integer& i) {
          return i.reveal<int64_t>(static_cast<int>(visibility));
        });
    return mapVectorToGroupedLiftMetrics(revealed);
  }

 private:
  pcf::Visibility visibility_;
};
} // namespace private_lift
