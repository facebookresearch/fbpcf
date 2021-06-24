/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

#include <emp-sh2pc/emp-sh2pc.h>

#include "fbpcf/common/FunctionalUtil.h"
#include "fbpcf/exception/PcfException.h"

namespace fbpcf {
template <typename EmpType>
class EmpVector {
 public:
  template <typename T>
  void add(const T& item) {
    throw PcfException{"Type doesn't match"};
  }

  template <typename T>
  void add(const std::vector<T>& v) {
    for (const auto i : v) {
      add(i);
    }
  }

  int64_t size() {
    return v_.size();
  }

  std::vector<EmpType> map(const std::function<EmpType(EmpType, EmpType)>& f) {
    return fbpcf::functional::map<std::pair<EmpType, EmpType>, EmpType>(
        v_, [&f](const auto& x) { return f(x.first, x.second); });
  }

 private:
  std::vector<std::pair<EmpType, EmpType>> v_;
};

template <>
template <>
inline void EmpVector<emp::Integer>::add<int64_t>(const int64_t& item) {
  const int32_t numBits = sizeof(int64_t) * 8;
  emp::Integer a{numBits, item, emp::ALICE};
  emp::Integer b{numBits, item, emp::BOB};
  v_.push_back(std::pair(a, b));
}

template <>
template <>
inline void EmpVector<emp::Bit>::add<bool>(const bool& item) {
  emp::Bit a{item, emp::ALICE};
  emp::Bit b{item, emp::BOB};
  v_.push_back(std::pair(a, b));
}
} // namespace fbpcf
