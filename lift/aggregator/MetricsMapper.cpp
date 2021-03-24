/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MetricsMapper.h"

namespace private_lift {
void addLiftMetricsToEmpVector(
    pcf::EmpVector<emp::Integer>& v,
    const LiftMetrics& metrics) {
  v.add(metrics.testPopulation);
  v.add(metrics.controlPopulation);
  v.add(metrics.testConversions);
  v.add(metrics.controlConversions);
  v.add(metrics.testConverters);
  v.add(metrics.controlConverters);
  v.add(metrics.testValue);
  v.add(metrics.controlValue);
  v.add(metrics.testSquared);
  v.add(metrics.controlSquared);
  v.add(metrics.testMatchCount);
  v.add(metrics.controlMatchCount);
  v.add(metrics.testImpressions);
  v.add(metrics.controlImpressions);
  v.add(metrics.testClicks);
  v.add(metrics.controlClicks);
  v.add(metrics.testSpend);
  v.add(metrics.controlSpend);
  v.add(metrics.testReach);
  v.add(metrics.controlReach);
}

LiftMetrics genLiftMetricsFromVector(
    const std::vector<int64_t>& v,
    int64_t& index) {
  LiftMetrics metrics;

  metrics.testPopulation = v[index++];
  metrics.controlPopulation = v[index++];
  metrics.testConversions = v[index++];
  metrics.controlConversions = v[index++];
  metrics.testConverters = v[index++];
  metrics.controlConverters = v[index++];
  metrics.testValue = v[index++];
  metrics.controlValue = v[index++];
  metrics.testSquared = v[index++];
  metrics.controlSquared = v[index++];
  metrics.testMatchCount = v[index++];
  metrics.controlMatchCount = v[index++];
  metrics.testImpressions = v[index++];
  metrics.controlImpressions = v[index++];
  metrics.testClicks = v[index++];
  metrics.controlClicks = v[index++];
  metrics.testSpend = v[index++];
  metrics.controlSpend = v[index++];
  metrics.testReach = v[index++];
  metrics.controlReach = v[index++];

  return metrics;
}

EncryptedLiftMetrics genLiftMetricsFromVector(
    const std::vector<emp::Integer>& v,
    int64_t& index) {
  EncryptedLiftMetrics metrics;

  metrics.testPopulation = v[index++];
  metrics.controlPopulation = v[index++];
  metrics.testConversions = v[index++];
  metrics.controlConversions = v[index++];
  metrics.testConverters = v[index++];
  metrics.controlConverters = v[index++];
  metrics.testValue = v[index++];
  metrics.controlValue = v[index++];
  metrics.testSquared = v[index++];
  metrics.controlSquared = v[index++];
  metrics.testMatchCount = v[index++];
  metrics.controlMatchCount = v[index++];
  metrics.testImpressions = v[index++];
  metrics.controlImpressions = v[index++];
  metrics.testClicks = v[index++];
  metrics.controlClicks = v[index++];
  metrics.testSpend = v[index++];
  metrics.controlSpend = v[index++];
  metrics.testReach = v[index++];
  metrics.controlReach = v[index++];

  return metrics;
}

void addLiftMetricsToEmpVector(
    std::vector<emp::Integer>& v,
    const EncryptedLiftMetrics& metrics) {
  v.push_back(metrics.testPopulation);
  v.push_back(metrics.controlPopulation);
  v.push_back(metrics.testConversions);
  v.push_back(metrics.controlConversions);
  v.push_back(metrics.testConverters);
  v.push_back(metrics.controlConverters);
  v.push_back(metrics.testValue);
  v.push_back(metrics.controlValue);
  v.push_back(metrics.testSquared);
  v.push_back(metrics.controlSquared);
  v.push_back(metrics.testMatchCount);
  v.push_back(metrics.controlMatchCount);
  v.push_back(metrics.testImpressions);
  v.push_back(metrics.controlImpressions);
  v.push_back(metrics.testClicks);
  v.push_back(metrics.controlClicks);
  v.push_back(metrics.testSpend);
  v.push_back(metrics.controlSpend);
  v.push_back(metrics.testReach);
  v.push_back(metrics.controlReach);
}

std::vector<emp::Integer> mapGroupedLiftMetricsToEmpVector(
    const GroupedEncryptedLiftMetrics& metrics) {
  std::vector<emp::Integer> v;

  addLiftMetricsToEmpVector(v, metrics.metrics);
  for (auto m : metrics.subGroupMetrics) {
    addLiftMetricsToEmpVector(v, m);
  }

  return v;
}

pcf::EmpVector<emp::Integer> mapGroupedLiftMetricsToEmpVector(
    const GroupedLiftMetrics& metrics) {
  pcf::EmpVector<emp::Integer> v;

  addLiftMetricsToEmpVector(v, metrics.metrics);
  for (auto m : metrics.subGroupMetrics) {
    addLiftMetricsToEmpVector(v, m);
  }

  return v;
}

GroupedLiftMetrics mapVectorToGroupedLiftMetrics(
    const std::vector<int64_t>& v) {
  GroupedLiftMetrics metrics;

  int64_t i = 0;
  metrics.metrics = genLiftMetricsFromVector(v, i);

  while (i < v.size()) {
    metrics.subGroupMetrics.push_back(genLiftMetricsFromVector(v, i));
  }

  return metrics;
}

GroupedEncryptedLiftMetrics mapVectorToGroupedLiftMetrics(
    const std::vector<emp::Integer>& v) {
  GroupedEncryptedLiftMetrics metrics;

  int64_t i = 0;
  metrics.metrics = genLiftMetricsFromVector(v, i);

  while (i < v.size()) {
    metrics.subGroupMetrics.push_back(genLiftMetricsFromVector(v, i));
  }

  return metrics;
}
} // namespace private_lift
