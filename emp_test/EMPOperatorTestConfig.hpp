/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace pcf {
enum EMPOperatorType { Addition };

struct EMPOperatorTestConfig {
  EMPOperatorType operatorType;
  int64_t inputData;
};
} // namespace pcf
