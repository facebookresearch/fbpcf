/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/util/AesPrg.h"
#include <string.h>
#include <stdexcept>

namespace fbpcf::engine::util {

inline uint64_t ceilDiv(uint64_t a, uint64_t b) {
  return a / b + (a % b != 0);
}

AesPrg::AesPrg(__m128i seed, int bufferSize)
    : cipher_(seed),
      prgCounter_(0),
      asyncBuffer_{std::make_unique<AsyncBuffer<unsigned char>>(
          sizeof(__m128i) * bufferSize,
          [this](uint64_t size) { return generateRandomData(size); })} {}

AesPrg::AesPrg(__m128i seed)
    : cipher_(seed), prgCounter_(0), asyncBuffer_(nullptr) {}

std::vector<bool> AesPrg::getRandomBits(uint32_t size) {
  if (!asyncBuffer_) {
    throw std::runtime_error("Can only generate random numbers in place!");
  }
  auto randomBytes = asyncBuffer_->getData(ceilDiv(size, 8));

  std::vector<bool> rst(size);
  for (auto i = 0; i < size; ++i) {
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

std::vector<unsigned char> AesPrg::generateRandomData(uint64_t numBytes) {
  std::vector<__m128i> rstM128i(ceilDiv(numBytes, sizeof(__m128i)));

  getRandomDataInPlace(rstM128i);

  std::vector<unsigned char> rstBytes(numBytes);
  memcpy(rstBytes.data(), rstM128i.data(), numBytes);
  return rstBytes;
}

} // namespace fbpcf::engine::util
