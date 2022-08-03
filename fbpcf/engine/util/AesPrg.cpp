/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/util/AesPrg.h"
#include <string.h>
#include <cstdint>
#include <stdexcept>

namespace fbpcf::engine::util {

AesPrg::AesPrg(__m128i seed, int bufferSize)
    : cipher_(seed),
      prgCounter_(0),
      asyncBuffer_{std::make_unique<AsyncBuffer<unsigned char>>(
          sizeof(__m128i) * bufferSize,
          [this](uint64_t size) {
            return std::async(
                [this](uint64_t size) { return generateRandomData(size); },
                size);
          })} {}

AesPrg::AesPrg(__m128i seed)
    : cipher_(seed), prgCounter_(0), asyncBuffer_(nullptr) {}

std::vector<bool> AesPrg::getRandomBits(uint32_t size) {
  if (!asyncBuffer_) {
    throw std::runtime_error("Can only generate random numbers in place!");
  }
  auto randomBytes = asyncBuffer_->getData(ceilDiv(size, 8));

  std::vector<bool> rst(size);
  for (size_t i = 0; i < size; ++i) {
    rst[i] = (randomBytes[i >> 3] >> (i & 7)) & 1;
  }
  return rst;
}

std::vector<unsigned char> AesPrg::getRandomBytes(uint32_t size) {
  if (!asyncBuffer_) {
    throw std::runtime_error("Can only generate random numbers in place!");
  }
  return asyncBuffer_->getData(size);
}

std::vector<uint64_t> AesPrg::getRandomUInt64(uint32_t size) {
  if (!asyncBuffer_) {
    throw std::runtime_error("Can only generate random numbers in place!");
  }
  auto randomBytes = asyncBuffer_->getData(size * 8);

  std::vector<uint64_t> rst(size);
  for (size_t i = 0; i < size; ++i) { // put random bytes into groups of 8 bytes
    for (size_t j = 0; j < 8; j++) {
      // add each byte to the end of the current numbers
      rst.at(i) = rst.at(i) << 8 ^ randomBytes.at((i << 3) ^ j);
    }
  }
  return rst;
}

std::vector<unsigned char> AesPrg::generateRandomData(uint64_t numBytes) {
  std::vector<__m128i> rstM128i(ceilDiv(numBytes, sizeof(__m128i)));

  getRandomDataInPlace(rstM128i);

  std::vector<unsigned char> rstBytes(numBytes);
  memcpy(rstBytes.data(), rstM128i.data(), numBytes);
  return rstBytes;
}

} // namespace fbpcf::engine::util
