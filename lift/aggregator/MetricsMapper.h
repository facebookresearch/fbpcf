#pragma once

#include <memory>

#include <emp-sh2pc/emp-sh2pc.h>

#include "../../pcf/mpc/EmpVector.h"
#include "../common/GroupedLiftMetrics.h"
#include "../common/GroupedEncryptedLiftMetrics.h"

namespace private_lift {
pcf::EmpVector<emp::Integer> mapGroupedLiftMetricsToEmpVector(
    const GroupedLiftMetrics& metrics);

GroupedLiftMetrics mapVectorToGroupedLiftMetrics(const std::vector<int64_t>& v);

GroupedEncryptedLiftMetrics mapVectorToGroupedLiftMetrics(
    const std::vector<emp::Integer>& v);

std::vector<emp::Integer> mapGroupedLiftMetricsToEmpVector(
    const GroupedEncryptedLiftMetrics& metrics);
} // namespace private_lift
