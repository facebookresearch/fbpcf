#pragma once

#include <memory>
#include <vector>

#include <folly/logging/xlog.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../common/GroupedLiftMetrics.h"
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

    XLOG(INFO) << "Aggregating metrics...";
    // Aggregate all metrics, returns std::vector<emp::Integer>
    auto v = pcf::functional::reduce<std::vector<emp::Integer>>(
        vv, pcf::vector::Add<emp::Integer>);

    XLOG(INFO) << "Revealing metrics...";
    XLOGF(DBG, "Visibility: {}", this->visibility_);
    // Reveal aggregated metrics
    auto revealed = pcf::functional::map<emp::Integer, int64_t>(
        v, [this](const emp::Integer& i) {
          return i.reveal<int64_t>(static_cast<int>(this->visibility_));
        });
    return mapVectorToGroupedLiftMetrics(revealed);
  }

 private:
  pcf::Visibility visibility_;
};
} // namespace private_lift
