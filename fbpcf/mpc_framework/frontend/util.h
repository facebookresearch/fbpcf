/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <type_traits>

namespace fbpcf::frontend {

// a class representing "signed integer"
template <int8_t intWidth>
struct Signed {
  static_assert(intWidth <= 64, "Can support at most 64-bit int");
  enum : int8_t { width = intWidth };
};

// a class representing "Unsigned integer"
template <int8_t intWidth>
struct Unsigned {
  static_assert(intWidth <= 64, "Can support at most 64-bit int");
  enum : int8_t { width = intWidth };
};

// helpers to indicate whether a integer is signed
template <typename T>
struct IsSigned;

template <int8_t width>
struct IsSigned<Unsigned<width>> : std::false_type {};

template <int8_t width>
struct IsSigned<Signed<width>> : std::true_type {};

// a batching class, indicating if a type is a batching
template <typename T>
struct Batch;

template <int8_t width>
struct Batch<Signed<width>> {
  using BasicType = Signed<width>;
};

template <int8_t width>
struct Batch<Unsigned<width>> {
  using BasicType = Unsigned<width>;
};

// a helper to identify if it's batching
template <typename T>
struct IsBatch;

template <int8_t width>
struct IsBatch<Signed<width>> : std::false_type {};

template <int8_t width>
struct IsBatch<Batch<Signed<width>>> : std::true_type {};

template <int8_t width>
struct IsBatch<Unsigned<width>> : std::false_type {};

template <int8_t width>
struct IsBatch<Batch<Unsigned<width>>> : std::true_type {};

template <typename T>
struct Secret {
  using type = T;
};

template <typename T>
struct Public {
  using type = T;
};

template <typename T>
struct IsSecret;

template <typename T>
struct IsSecret<Secret<T>> : std::true_type {};

template <typename T>
struct IsSecret<Public<T>> : std::false_type {};

template <typename OutputT, typename InputT1, typename InputT2>
void equalityCheck(OutputT& rst, const InputT1& src1, const InputT2& src2) {
  // first compute XOR and NOT gates
  for (int8_t i = 0; i < src1.size(); i++) {
    rst[i] = (!src1.at(i) ^ src2.at(i));
  }
  // compute AND gates in pairs and store in subarray of rst with size tmpWidth
  int8_t tmpWidth = src1.size();
  while (tmpWidth > 1) {
    for (int8_t i = 0; i < tmpWidth / 2; i++) {
      rst[i] = rst.at(i) & rst.at(tmpWidth - i - 1);
    }
    tmpWidth -= tmpWidth / 2;
  }
}

} // namespace fbpcf::frontend
