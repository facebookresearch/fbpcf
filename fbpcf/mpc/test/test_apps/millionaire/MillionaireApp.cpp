/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "folly/Random.h"

#include "./MillionaireApp.h"

namespace fbpcf {

int MillionaireApp::getInputData() {
  return folly::Random::rand32(0, 1000000000);
}

void MillionaireApp::putOutputData(const bool&) {}
} // namespace fbpcf
