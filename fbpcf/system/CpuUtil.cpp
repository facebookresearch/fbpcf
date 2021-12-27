/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CpuUtil.h"

#include <cstring>

namespace fbpcf::system {
// reference: https://bduvenhage.me/rng/2019/04/06/the-intel-drng.html
CpuId getCpuId(const uint64_t& eax) {
  CpuId info;
  asm volatile("cpuid"
               : "=a"(info.eax), "=b"(info.ebx), "=c"(info.ecx), "=d"(info.edx)
               : "a"(eax), "c"(0));

  return info;
}

bool isIntelCpu() {
  auto info = getCpuId(0);
  return !(
      std::memcmp((char*)&info.ebx, "Genu", 4) ||
      std::memcmp((char*)&info.ecx, "ntel", 4) ||
      std::memcmp((char*)&info.edx, "ineI", 4));
}

bool isDrngSupported() {
  bool rdrandSupported = false;
  bool rdseedSupported = false;

  if (isIntelCpu()) {
    auto info = getCpuId(1);
    if ((info.ecx & 0x40000000) == 0x40000000) {
      rdrandSupported = true;
    }

    info = getCpuId(7);
    if ((info.ebx & 0x40000) == 0x40000) {
      rdseedSupported = true;
    }
  }

  return rdrandSupported && rdseedSupported;
}
} // namespace fbpcf::system
