/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace fbpcf::mpc_std_lib::oram {

class IFilter {
 public:
  enum FilterType { GT, LT, GTE, LTE, EQ, NEQ, SUBSET_OF, NOT_SUBSET_OF };

  virtual ~IFilter() = default;

  // Returns true if the filter condition passes
  virtual bool apply(const std::vector<uint32_t>& breakdownValues) const = 0;
};

class SingleValueFilter : public IFilter {
 public:
  explicit SingleValueFilter(
      FilterType type,
      size_t columnIndex,
      uint32_t filterValue)
      : type_{type}, columnIndex_{columnIndex}, filterValue_{filterValue} {
    switch (type_) {
      case GT:
      case LT:
      case GTE:
      case LTE:
      case EQ:
      case NEQ:
        break;
      case SUBSET_OF:
      case NOT_SUBSET_OF:
        throw std::invalid_argument(
            "SingleValueFilter must be of type (GT, LT, GTE, LTE, EQ, NEQ");
    }
  }

  bool apply(const std::vector<uint32_t>& breakdownValues) const override {
    if (breakdownValues.size() < columnIndex_ + 1) {
      throw std::invalid_argument(
          "Column index of filter exceeds number of breakdown values. Expected at least " +
          std::to_string(columnIndex_ + 1) + " values but got " +
          std::to_string(breakdownValues.size()));
    }

    switch (type_) {
      case GT:
        return breakdownValues[columnIndex_] > filterValue_;
      case LT:
        return breakdownValues[columnIndex_] < filterValue_;
      case GTE:
        return breakdownValues[columnIndex_] >= filterValue_;
      case LTE:
        return breakdownValues[columnIndex_] <= filterValue_;
      case EQ:
        return breakdownValues[columnIndex_] == filterValue_;
      case NEQ:
        return breakdownValues[columnIndex_] != filterValue_;
      case SUBSET_OF:
      case NOT_SUBSET_OF:
        throw std::invalid_argument(
            "SingleValueFilter must be of type (GT, LT, GTE, LTE, EQ, NEQ");
    }
  }

  // Filter condition operator
  FilterType type_;
  // The column index the filter applies to
  size_t columnIndex_;
  // The comparison value for the filter
  uint32_t filterValue_;
};

class VectorValueFilter : public IFilter {
 public:
  explicit VectorValueFilter(
      FilterType type,
      size_t columnIndex,
      std::vector<uint32_t> filterValues)
      : type_{type},
        columnIndex_{columnIndex},
        filterValues_{filterValues.begin(), filterValues.end()} {
    switch (type) {
      case GT:
      case LT:
      case GTE:
      case LTE:
      case EQ:
      case NEQ:
        throw std::invalid_argument(
            "VectorValueFilter must be of type (SUBSET_OF, NOT_SUBSET_OF)");
      case SUBSET_OF:
      case NOT_SUBSET_OF:
        break;
    }
  }

  bool apply(const std::vector<uint32_t>& breakdownValues) const override {
    if (breakdownValues.size() < columnIndex_ + 1) {
      throw std::invalid_argument(
          "Column index of filter exceeds number of breakdown values. Expected at least " +
          std::to_string(columnIndex_ + 1) + " values but got " +
          std::to_string(breakdownValues.size()));
    }

    switch (type_) {
      case GT:
      case LT:
      case GTE:
      case LTE:
      case EQ:
      case NEQ:
        throw std::invalid_argument(
            "VectorValueFilter must be of type (SUBSET_OF NOT_SUBSET_OF)");
      case SUBSET_OF:
        return filterValues_.find(breakdownValues[columnIndex_]) !=
            filterValues_.end();
      case NOT_SUBSET_OF:
        return filterValues_.find(breakdownValues[columnIndex_]) ==
            filterValues_.end();
    }
  }

  // Type of multi value filter
  FilterType type_;
  // The column index the filter applies to
  size_t columnIndex_;
  // List of values for the multi value filter
  std::set<uint32_t> filterValues_;
};

} // namespace fbpcf::mpc_std_lib::oram
