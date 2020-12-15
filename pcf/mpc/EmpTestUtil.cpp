#include "EmpTestUtil.h"

#include "../system/CpuUtil.h"

namespace pcf::mpc {
bool isTestable() {
#ifndef ENABLE_RDSEED
  return true;
#else
  return isDrngSupported();
#endif
}
} // namespace pcf::mpc
