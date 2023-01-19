/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <smmintrin.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "fbpcf/engine/util/aes.h"
#include "folly/logging/xlog.h"

namespace fbpcf::engine::util {
inline bool getLsb(__m128i src) {
  return _mm_extract_epi8(src, 0) & 1;
}

inline bool getMsb(__m128i src) {
  return (_mm_extract_epi8(src, 15) >> 7);
}

/*
 * Left shift a value in m128i by an arbitery(offset) bits. This is needed as
 * there's no instruction for this functionality AFAIK.
 */
inline void lShiftByBitsInPlace(__m128i& src, int offset) {
  __m128i v1, v2;
  if (offset >= 64) {
    src = _mm_slli_si128(src, 8);
    src = _mm_slli_epi64(src, offset - 64);
  } else {
    v1 = _mm_slli_epi64(src, offset);
    v2 = _mm_slli_si128(src, 8);
    v2 = _mm_srli_epi64(v2, 64 - offset);
    src = _mm_or_si128(v1, v2);
  }
}

inline uint64_t getLast64Bits(__m128i src) {
  return _mm_extract_epi64(src, 0);
}
/*
 * Extracts the last n bits in a 128 bit integer to the passed in boolean
 * vector. The order of bits will be from least significant to most
 * significant.
 */
inline void extractLnbToVector(const __m128i& src, std::vector<bool>& data) {
  if (data.size() > sizeof(src) * 8) {
    throw std::runtime_error("Vector size is larger than __m128i size");
  }
  uint64_t lower64 = _mm_extract_epi64(src, 0);
  uint64_t upper64 = _mm_extract_epi64(src, 1);

  size_t lowerBitsToCopy = std::min(sizeof(lower64) * 8, data.size());
  size_t upperBitsToCopy = data.size() - lowerBitsToCopy;
  for (size_t i = 0; i < lowerBitsToCopy; i++) {
    data[i] = lower64 & 1;
    lower64 = lower64 >> 1;
  }
  for (size_t i = 0; i < upperBitsToCopy; i++) {
    data[i + sizeof(lower64) * 8] = upper64 & 1;
    upper64 = upper64 >> 1;
  }
}

inline void setLsbTo0(__m128i& src) {
  src = _mm_andnot_si128(_mm_set_epi64x(0, 1), src);
}

inline void setLsbTo1(__m128i& src) {
  src = _mm_or_si128(_mm_set_epi64x(0, 1), src);
}

inline __m128i buildM128i(const std::vector<unsigned char>& src) {
  return _mm_set_epi8(
      src[15],
      src[14],
      src[13],
      src[12],
      src[11],
      src[10],
      src[9],
      src[8],
      src[7],
      src[6],
      src[5],
      src[4],
      src[3],
      src[2],
      src[1],
      src[0]);
}

class EngineWrap {
 public:
  EngineWrap() {
    OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS, nullptr);
    ENGINE_load_rdrand();

    engine_ = ENGINE_by_id("rdrand");
    if (engine_ == nullptr) {
      throw std::runtime_error(
          "System doesn't support hardware PRG. Error code: " +
          std::to_string(ERR_get_error()));
    }
    if (ENGINE_init(engine_) == 0) {
      throw std::runtime_error(
          "Failed to initialize PRG engine. Error code: " +
          std::to_string(ERR_get_error()));
    }
    if (ENGINE_set_default(engine_, ENGINE_METHOD_RAND) == 0) {
      throw std::runtime_error(
          "ENGINE_set_default failed. Error code: " +
          std::to_string(ERR_get_error()));
    }
    XLOG(INFO) << "Successfully initialized the hardware PRG.";
  }
  ~EngineWrap() {
    if (engine_) {
      ENGINE_finish(engine_);
      ENGINE_free(engine_);
      ENGINE_cleanup();
    }
  }

 private:
  // can't use smart pointer due to the complicated deallocation operation.
  ENGINE* engine_;
};

inline __m128i getRandomM128iFromSystemNoise() {
  // use this variable to trigger the initialization only once globally.
  static const EngineWrap engine;
  std::vector<unsigned char> buf(sizeof(__m128i));
  // RAND_bytes is provided by openssl
  if (RAND_bytes(reinterpret_cast<uint8_t*>(buf.data()), sizeof(__m128i)) !=
      1) {
    throw std::runtime_error("No sufficient entropy to initialize the seed.");
  }
  return buildM128i(buf);
}

inline void loadBytesIntoBignum(
    const std::vector<unsigned char>& data,
    BIGNUM* num) {
  for (size_t i = data.size(); i > 0; i--) {
    if (!BN_lshift(num, num, 8)) {
      throw std::runtime_error(
          "BN_lshift failed at index " + std::to_string(i) +
          " Error:  " + std::to_string(ERR_get_error()));
    }
    if (!BN_add_word(num, data[i - 1])) {
      throw std::runtime_error(
          "BN_add_word failed at index " + std::to_string(i) +
          " Error:  " + std::to_string(ERR_get_error()));
    }
  }
}

inline uint32_t
mod(const std::vector<unsigned char>& data, uint32_t modulo, BN_CTX* ctx) {
  BN_CTX_start(ctx);
  BIGNUM* num = BN_CTX_get(ctx);

  if (num == nullptr) {
    throw std::runtime_error(
        "Temp BIGNUM initialization failed: " +
        std::to_string(ERR_get_error()));
  }

  loadBytesIntoBignum(data, num);

  BN_ULONG rst = BN_mod_word(num, modulo);
  BN_ULONG bn_zero = 0;

  // invalidates num
  BN_CTX_end(ctx);

  if (rst == (bn_zero - 1)) {
    throw std::runtime_error(
        "BIGNUM mod failed: " + std::to_string(ERR_get_error()));
  }
  return rst;
}

// this class is to determine the role in OT/RCOT/MPCOT/SPCOT where the
// role of the two parties are not symmetric.
enum Role {
  sender,
  receiver,
};

/**
 * This class expand an array of n keys into an array of 2n keys
 * The i-th key in the input array controls the 2i-th, (2i+1)-th keys in the
 * output array.
 */
class Expander {
 public:
  explicit Expander(int64_t index);
  std::vector<__m128i> expand(std::vector<__m128i>&& src) const;

 private:
  Aes cipher0_;
  Aes cipher1_;
};

} // namespace fbpcf::engine::util
