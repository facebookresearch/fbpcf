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
   * Boolean version of composite multiplicative triple. Rather than being a
   * single a and c however, there are n 'a' values and n 'c' values.
   * For each 0 <= i < n, ai & b = ci. I.e. it holds n regular boolean tuples
   * with the b bit shared.
   *
   */
  class CompositeBooleanTuple {
   public:
    CompositeBooleanTuple() {}

    CompositeBooleanTuple(bool a, std::vector<bool> b, std::vector<bool> c)
        : a_{a}, b_{b}, c_{c} {
      if (b.size() != c.size()) {
        throw std::invalid_argument("Sizes of a and c must be equal");
      }
    }

    // get the secret-share of shared bit A
    bool getA() {
      return a_;
    }

    // get the vector of B bit shares
    std::vector<bool> getB() {
      return b_;
    }

    // get the vector of C bit shares
    std::vector<bool> getC() {
      return c_;
    }

   private:
    bool a_;
    std::vector<bool> b_, c_;
  };

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
   * Generate a number of boolean tuples.
   * @param size number of tuples to generate.
   */
  virtual std::vector<BooleanTuple> getBooleanTuple(uint32_t size) = 0;

  /**
   * Generate a number of composite boolean tuples.
   * @param tupleSize A map of tuple sizes requested to the number of those
   * tuples to generate.
   * @return A map of tuple sizes to vector of those tuples
   */
  virtual std::map<size_t, std::vector<CompositeBooleanTuple>>
  getCompositeTuple(const std::map<size_t, uint32_t>& tupleSizes) = 0;

  /**
   * Wrapper method for getBooleanTuple() and getCompositeTuple() which performs
   * only one round of communication.
   */
  virtual std::pair<
      std::vector<BooleanTuple>,
      std::map<size_t, std::vector<CompositeBooleanTuple>>>
  getNormalAndCompositeBooleanTuples(
      uint32_t tupleSize,
      const std::map<size_t, uint32_t>& compositeTupleSizes) = 0;

  /**
   * Temporary method to indicate whether it's safe to call composite tuple
   * generation methods
   */
  virtual bool supportsCompositeTupleGeneration() = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator
