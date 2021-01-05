/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <functional>

#include "../perf/PerfUtil.h"

namespace pcf {
template <class InputDataType, class OutputDataType>
class IMpcGame {
 public:
  virtual ~IMpcGame(){};

  OutputDataType perfPlay(const InputDataType& inputData) {
    auto decorator = pcf::perf::decorate(
        "IMpcGame::perfPlay",
        static_cast<
            std::function<OutputDataType(IMpcGame*, const InputDataType&)>>(
            &IMpcGame::play));
    return decorator(this, inputData);
  }

  virtual OutputDataType play(const InputDataType& inputData) = 0;
};
} // namespace pcf
