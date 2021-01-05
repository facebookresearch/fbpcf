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
  v.add(metrics.testBuyers);
  v.add(metrics.controlBuyers);
  v.add(metrics.testSales);
  v.add(metrics.controlSales);
  v.add(metrics.testSquared);
  v.add(metrics.controlSquared);
}

LiftMetrics genLiftMetricsFromVector(
    const std::vector<int64_t>& v,
    int& index) {
  LiftMetrics metrics;

  metrics.testPopulation = v[index++];
  metrics.controlPopulation = v[index++];
  metrics.testBuyers = v[index++];
  metrics.controlBuyers = v[index++];
  metrics.testSales = v[index++];
  metrics.controlSales = v[index++];
  metrics.testSquared = v[index++];
  metrics.controlSquared = v[index++];

  return metrics;
}

EncryptedLiftMetrics genLiftMetricsFromVector(
    const std::vector<emp::Integer>& v,
    int& index) {
  EncryptedLiftMetrics metrics;

  metrics.testPopulation = v[index++];
  metrics.controlPopulation = v[index++];
  metrics.testBuyers = v[index++];
  metrics.controlBuyers = v[index++];
  metrics.testSales = v[index++];
  metrics.controlSales = v[index++];
  metrics.testSquared = v[index++];
  metrics.controlSquared = v[index++];

  return metrics;
}

void addLiftMetricsToEmpVector(
    std::vector<emp::Integer>& v,
    const EncryptedLiftMetrics& metrics) {
  v.push_back(metrics.testPopulation);
  v.push_back(metrics.controlPopulation);
  v.push_back(metrics.testBuyers);
  v.push_back(metrics.controlBuyers);
  v.push_back(metrics.testSales);
  v.push_back(metrics.controlSales);
  v.push_back(metrics.testSquared);
  v.push_back(metrics.controlSquared);
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

  int i = 0;
  metrics.metrics = genLiftMetricsFromVector(v, i);

  while (i < v.size()) {
    metrics.subGroupMetrics.push_back(genLiftMetricsFromVector(v, i));
  }

  return metrics;
}

GroupedEncryptedLiftMetrics mapVectorToGroupedLiftMetrics(
    const std::vector<emp::Integer>& v) {
  GroupedEncryptedLiftMetrics metrics;

  int i = 0;
  metrics.metrics = genLiftMetricsFromVector(v, i);

  while (i < v.size()) {
    metrics.subGroupMetrics.push_back(genLiftMetricsFromVector(v, i));
  }

  return metrics;
}
} // namespace private_lift
