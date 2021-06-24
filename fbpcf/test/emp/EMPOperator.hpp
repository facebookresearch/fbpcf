/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc/EmpGame.h"
#include "fbpcf/mpc/QueueIO.h"

namespace fbpcf {

template <class DataType, class EmpDataType, typename InputConfig>
class EMPOperator : public EmpGame<QueueIO, InputConfig, DataType> {
 public:
  EMPOperator(std::unique_ptr<QueueIO> io, Party party)
      : EmpGame<QueueIO, InputConfig, DataType>(std::move(io), party) {}

  DataType play(const InputConfig& input) {
    EmpDataType emp_res =
        operate<DataType, EmpDataType>(input.inputData, input.op);
    DataType result = emp_res.template reveal<DataType>();
    XLOGF(INFO, "result of Alice and Bob:  {}", result);
    return result;
  }

 private:
  template <typename I, typename T>
  T operate(const I& input, std::function<T(T, T)> f) {
    return operate(input, f);
  }

  template <>
  emp::Integer operate<int64_t, emp::Integer>(
      const int64_t& input,
      std::function<emp::Integer(emp::Integer, emp::Integer)> f) {
    emp::Integer a{64, input, emp::ALICE};
    emp::Integer b{64, input, emp::BOB};
    return f(a, b);
  }

  template <>
  emp::Bit operate<bool, emp::Bit>(
      const bool& input,
      std::function<emp::Bit(emp::Bit, emp::Bit)> f) {
    emp::Bit a{input, emp::ALICE};
    emp::Bit b{input, emp::BOB};
    return f(a, b);
  }
};
} // namespace fbpcf
