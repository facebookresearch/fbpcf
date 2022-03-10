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

#include "fbpcf/mpc_framework/engine/util/aes.h"
#include "folly/logging/xlog.h"

namespace fbpcf::engine::util {

inline bool getLsb(__m128i src) {
  return _mm_extract_epi8(src, 0) & 1;
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
