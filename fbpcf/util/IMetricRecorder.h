/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/dynamic.h>

namespace fbpcf::util {
/**
 * Different components could have different implementation of this recorder.
 * The only function of this interface is to allow metric collector to pull
 * metrics regardless of the actual implementation. The implementation of
 * this object needs to be thread-safe.
 */

class IMetricRecorder {
 public:
  virtual ~IMetricRecorder() = default;
  // return a map of metric name and values.
  virtual folly::dynamic getMetrics() const = 0;
};

} // namespace fbpcf::util
