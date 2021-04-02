/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <filesystem>
#include <vector>

#include <gflags/gflags.h>

#include "../../pcf/mpc/EmpGame.h"
#include "CalculatorGameConfig.h"
#include "InputData.h"
#include "OutputMetrics.h"

namespace private_lift {
template <class IOChannel>
class CalculatorGame
    : public pcf::EmpGame<IOChannel, CalculatorGameConfig, std::string> {
 public:
  CalculatorGame(
      std::unique_ptr<IOChannel> ioChannel,
      pcf::Party party,
      pcf::Visibility visibility = pcf::Visibility::Public)
      : pcf::EmpGame<IOChannel, CalculatorGameConfig, std::string>(
            std::move(ioChannel),
            party),
        visibility_{visibility},
        party_{party} {}

  std::string play(const CalculatorGameConfig& config) override {
    std::string output = "";
    /*
     * Run the Conversion Lift circuit which will compute Lift metrics for the
     * overall dataset plus all found cohorts. Output is printed to stdout for
     * verification. The overall metrics are returned.
     * TODO: cohortOut should also be returned or sent to a file.
     * We currently have no way of returning cohort metrics to the caller.
     */
    if (party_ == pcf::Party::Alice) {
      OutputMetrics<PUBLISHER> outputMetrics{
          config.inputData,
          config.isConversionLift,
          visibility_ == pcf::Visibility::Xor,
          config.numConversionsPerUser};
      output = outputMetrics.playGame();
    } else {
      OutputMetrics<PARTNER> outputMetrics{config.inputData,
                                           config.isConversionLift,
                                           visibility_ == pcf::Visibility::Xor,
                                           config.numConversionsPerUser};
      output = outputMetrics.playGame();
    }
    return output;
  }

 private:
  pcf::Visibility visibility_;
  pcf::Party party_;
};
} // namespace private_lift
