/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "EmpTestUtil.h"

#include "../system/CpuUtil.h"

namespace fbpcf::mpc {
bool isTestable() {
#ifndef ENABLE_RDSEED
  return true;
#else
  return isDrngSupported();
#endif
}
} // namespace fbpcf::mpc
