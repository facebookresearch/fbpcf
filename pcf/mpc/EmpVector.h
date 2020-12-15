#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

#include <emp-sh2pc/emp-sh2pc.h>

#include "../common/FunctionalUtil.h"
#include "../exception/PcfException.h"

namespace pcf {
template <typename EmpType>
class EmpVector {
 public:
  template <typename T>
  void add(const T& item) {
    throw PcfException{"Type doesn't match"};
  }

  template <>
  void add<int64_t>(const int64_t& item) {
    if (!std::is_same<EmpType, emp::Integer>::value) {
      throw PcfException{"Type doesn't match"};
    }
    const int numBits = sizeof(int64_t) * 8;
    EmpType a{numBits, item, emp::ALICE};
    EmpType b{numBits, item, emp::BOB};
    v_.push_back(std::pair(a, b));
  }

  template <>
  void add<bool>(const bool& item) {
    if (!std::is_same<EmpType, emp::Bit>::value) {
      throw PcfException{"Type doesn't match"};
    }
    EmpType a{item, emp::ALICE};
    EmpType b{item, emp::BOB};
    v_.push_back(std::pair(a, b));
  }

  template <typename T>
  void add(const std::vector<T>& v) {
    for (const auto i : v) {
      add(i);
    }
  }

  int size() {
    return v_.size();
  }

  std::vector<EmpType> map(const std::function<EmpType(EmpType, EmpType)>& f) {
    return pcf::functional::map<std::pair<EmpType, EmpType>, EmpType>(
        v_, [&f](const auto& x) { return f(x.first, x.second); });
  }

 private:
  std::vector<std::pair<EmpType, EmpType>> v_;
};
} // namespace pcf
