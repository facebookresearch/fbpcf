/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>

namespace fbpcf::engine::tuple_generator {

/**
 The integer tuple generator API
 */
class IArithmeticTupleGenerator {
 public:
  virtual ~IArithmeticTupleGenerator() = default;

  /**
   * This is a integer version multiplicative triple. This object represents the
   * shares hold by one party, e.g. the share of a, b and their product c.
   */
  class IntegerTuple {
   public:
    IntegerTuple() {}

    IntegerTuple(uint64_t a, uint64_t b, uint64_t c) : a_(a), b_(b), c_(c) {}

    // get the first share
    uint64_t getA() const {
      return a_;
    }

    // get the second share
    uint64_t getB() const {
      return b_;
    }

    // get the third share
    uint64_t getC() const {
      return c_;
    }

   private:
    uint64_t a_, b_, c_;
  };

  /**
   * Generate a number of integer tuples.
   * @param size number of tuples to generate.
   */
  virtual std::vector<IntegerTuple> getIntegerTuple(uint32_t size) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator
