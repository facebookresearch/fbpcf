/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/dynamic.h>
#include <folly/logging/xlog.h>
#include <string>
#include <unordered_map>
#include "fbpcf/util/IMetricRecorder.h"

namespace fbpcf::util {

class MetricCollector {
 public:
  explicit MetricCollector(std::string prefix) : prefix_(prefix) {}

  void addNewRecorder(
      std::string name,
      std::shared_ptr<IMetricRecorder> recorder) {
    recorders_.emplace(name, recorder);
  }

  folly::dynamic collectMetrics() const {
    folly::dynamic result = folly::dynamic::object;
    for (const auto& [key, recorder] : recorders_) {
      result.insert(prefix_ + "." + key, recorder->getMetrics());
    }
    return result;
  }

  folly::dynamic aggregateMetrics() const {
    folly::dynamic result = folly::dynamic::object;

    for (const auto& [key, recorder] : recorders_) {
      auto&& metrics = recorder->getMetrics();
      for (const auto& [metricKey, metricValue] : metrics.items()) {
        if (metricValue.isNumber()) {
          auto aggKey = prefix_ + "." + metricKey.asString();

          if (result.count(aggKey)) {
            result[aggKey] += metricValue;
          } else {
            result.insert(aggKey, metricValue);
          }
        }
      }
    }

    return result;
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<IMetricRecorder>> recorders_;
  std::string prefix_;
};

} // namespace fbpcf::util
