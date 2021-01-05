/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

#include <glog/logging.h>

#include "../../pcf/mpc/EmpApp.h"
#include "../../pcf/mpc/EmpGame.h"
#include "CalculatorGame.h"
#include "InputData.h"
#include "OutputMetrics.h"

namespace private_lift {

class CalculatorApp : public pcf::EmpApp<
                          CalculatorGame<emp::NetIO>,
                          CalculatorGameConfig,
                          std::string> {
 public:
  CalculatorApp(
      const pcf::Party party,
      const std::string& serverIp,
      const uint16_t port,
      const std::filesystem::path& inputPath,
      const std::string& outputPath,
      const bool useXorEncryption)
      : pcf::EmpApp<
            CalculatorGame<emp::NetIO>,
            CalculatorGameConfig,
            std::string>{party, serverIp, port},
        inputPath_(inputPath),
        outputPath_(outputPath),
        visibility_(
            useXorEncryption ? pcf::Visibility::Xor : pcf::Visibility::Public) {
  }

  void run() override;

 protected:
  CalculatorGameConfig getInputData() override;
  void putOutputData(const std::string& output) override;

 private:
  std::filesystem::path inputPath_;
  std::string outputPath_;
  pcf::Visibility visibility_;
};

} // namespace private_lift

#include "CalculatorApp.hpp"
