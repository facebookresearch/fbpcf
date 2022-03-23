/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <xmmintrin.h>
#include <algorithm>
#include <array>
#include <future>
#include <stdexcept>

#include <memory>
#include <thread>
#include <vector>

#include <fbpcf/engine/util/util.h>
#include "fbpcf/engine/util/AsyncBuffer.h"
#include "fbpcf/engine/util/IPrg.h"
#include "fbpcf/engine/util/aes.h"

namespace fbpcf::engine::util {

class AesPrg final : public IPrg {
 public:
  /**
   * Create a prg that has an async buffer and can continuously generate random
   * numbers.
   */
  AesPrg(__m128i seed, int bufferSize);

  /**
   * Create a prg that doesn't have an async buffer and can only generate random
   * numbers in place.
   */
  explicit AesPrg(__m128i seed);

  /**
   * @inherit doc
   * If number of bits is not a product of 8, then some extra random bits will
   * be generated to make them full bytes. The extra bits will be discarded.
   * Therefore, this implementation doesn't guarantee the output of
   * getRandomBits() and getRandomBytes() will always generate the same output.
   */
  std::vector<bool> getRandomBits(uint32_t size) override;

  /**
   * @inherit doc
   */
  std::vector<unsigned char> getRandomBytes(uint32_t size) override;

  inline void getRandomDataInPlace(std::vector<__m128i>& data) {
    // this can happen only if there is about to be an overflow.
    if (prgCounter_ > 0xFFFFFFFFFFFFFFFF /* 2^ 64 - 1 */ - data.size()) {
      throw std::runtime_error("PRG counter overflow!");
    }
    for (size_t i = 0; i < data.size(); i++) {
      data[i] = _mm_set_epi64x(0, prgCounter_++);
    }
    cipher_.encryptInPlace(data);
  }

  inline void getRandomBitsInPlace(std::vector<bool>& data) {
    std::vector<__m128i> aesResults(ceilDiv(data.size(), sizeof(__m128i) * 8));
    getRandomDataInPlace(aesResults);

    std::vector<bool> extractedBits(sizeof(__m128i) * 8);
    for (size_t iEncrypted = 0; iEncrypted < aesResults.size(); iEncrypted++) {
      int bitsToCopy = std::min(
          data.size() - iEncrypted * sizeof(__m128i) * 8, sizeof(__m128i) * 8);
      util::extractLnbToVector(aesResults[iEncrypted], extractedBits);
      for (int iBits = 0; iBits < bitsToCopy; iBits++) {
        data[iEncrypted * sizeof(__m128i) * 8 + iBits] = extractedBits[iBits];
      }
    }
  }

 private:
  inline uint64_t ceilDiv(uint64_t a, uint64_t b) {
    return a / b + (a % b != 0);
  }
  inline std::vector<unsigned char> generateRandomData(uint64_t numBytes);

  Aes cipher_;

  uint64_t prgCounter_;

  std::unique_ptr<AsyncBuffer<unsigned char>> asyncBuffer_ = nullptr;
};

} // namespace fbpcf::engine::util
