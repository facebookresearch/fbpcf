/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <vector>

namespace fbpcf::engine::tuple_generator {

const uint64_t kDefaultBufferSize = 16384;

/**
 The boolean tuple generator API
 */
class ITupleGenerator {
 public:
  virtual ~ITupleGenerator() = default;

  /**
   * This is a boolean version multiplicative triple. This object represents the
   * shares hold by one party, e.g. the share of a, b and their product c.
   * This implementation is space-efficient, such that the 3 bits are compressed
   * into one byte for storage.
   */
  class BooleanTuple {
   public:
    BooleanTuple() {}

    BooleanTuple(bool a, bool b, bool c) {
      value_ = (a << 2) ^ (b << 1) ^ c;
    }

    // get the first share
    bool getA() const {
      return (value_ >> 2) & 1;
    }

    // get the second share
    bool getB() const {
      return (value_ >> 1) & 1;
    }

    // get the third share
    bool getC() const {
      return value_ & 1;
    }

   private:
    // a, b, c takes the last 3 bits to store.
    unsigned char value_;
  };

  /**
   * Generate a number of boolean tuples.
   * @param size number of tuples to generate.
   */
  virtual std::vector<BooleanTuple> getBooleanTuple(uint32_t size) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator
