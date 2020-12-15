#pragma once

#include <stdint.h>

namespace pcf::system {
struct CpuId {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
};

CpuId getCpuId(const uint64_t& eax);
bool isIntelCpu();
bool isDrngSupported();
} // namespace pcf::system
