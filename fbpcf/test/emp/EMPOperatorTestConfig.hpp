/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

namespace fbpcf {

template <typename DataType, typename EmpDataType>
struct EMPOperatorTestConfig {
  std::function<EmpDataType(EmpDataType, EmpDataType)> op;
  DataType inputData;
};
} // namespace fbpcf
