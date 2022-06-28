/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include <folly/dynamic.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::engine::communication {

/**
 * An communication factory API
 */
class IPartyCommunicationAgentFactory {
 public:
  explicit IPartyCommunicationAgentFactory(std::string name)
      : metricCollector_(std::make_shared<fbpcf::util::MetricCollector>(name)) {
  }

  virtual ~IPartyCommunicationAgentFactory() = default;

  /**
   * create an agent that talks to a certain party.
   */
  virtual std::unique_ptr<IPartyCommunicationAgent> create(
      int id,
      std::string name) = 0;

  std::shared_ptr<fbpcf::util::MetricCollector> getMetricsCollector() const {
    return metricCollector_;
  }

 protected:
  std::shared_ptr<fbpcf::util::MetricCollector> metricCollector_;
};

} // namespace fbpcf::engine::communication
