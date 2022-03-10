/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <vector>

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

/**
 * This object is to hide the complicated matrix multiplication, which is
 * necessary to meet the requirements of LPN assumption, from the caller. The
 * matrix is generated via a code generator. See details at
 * https://eprint.iacr.org/2020/924.pdf
 */
class IMatrixMultiplier {
 public:
  virtual ~IMatrixMultiplier() = default;

  /**
   * multiply the src with a randomly generated matrix with seed
   * @param seed the seed to generate the matrix
   * @param rstLength the length of the result vector
   * @param src the vector for matrix multiplication
   * @return the product
   */
  virtual std::vector<__m128i> multiplyWithRandomMatrix(
      __m128i seed,
      int64_t dstLength,
      const std::vector<__m128i>& src) const = 0;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
