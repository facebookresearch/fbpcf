/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emp-sh2pc/emp-sh2pc.h>

#include "./MillionaireGame.h" // @manual
#include "fbpcf/mpc/EmpApp.h"

namespace fbpcf {
class MillionaireApp : public EmpApp<MillionaireGame<emp::NetIO>, int, bool> {
 public:
  MillionaireApp(Party party, const std::string& serverIp, uint16_t port)
      : EmpApp<MillionaireGame<emp::NetIO>, int, bool>{party, serverIp, port} {}

 protected:
  int getInputData() override;
  void putOutputData(const bool& output) override;
};
} // namespace fbpcf
