/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "folly/logging/xlog.h"
#include "../pcf/mpc/EmpGame.h"
#include "EMPOperatorTestConfig.hpp"

namespace pcf {

template <class IOChannel, class OutputDataType>
class EMPOperator
    : public EmpGame<IOChannel, EMPOperatorTestConfig, OutputDataType> {
 public:
  EMPOperator(std::unique_ptr<IOChannel> io, Party party)
      : EmpGame<IOChannel, EMPOperatorTestConfig, OutputDataType>(
            std::move(io),
            party) {}
  OutputDataType play(const EMPOperatorTestConfig& inputConfig) {
    emp::Integer alice{64, inputConfig.inputData, emp::ALICE};
    emp::Integer bob{64, inputConfig.inputData, emp::BOB};

    OutputDataType result;
    switch (inputConfig.operatorType) {
      case EMPOperatorType::Addition:
        result = (alice + bob).reveal<OutputDataType>();
        break;
    }

    XLOGF(
        INFO,
        "{} result of Alice and Bob:  {}",
        inputConfig.operatorType,
        result);
    return result;
  }
};
} // namespace pcf
