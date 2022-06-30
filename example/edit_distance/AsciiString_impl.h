/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>
#include "./AsciiString.h" // @manual

#pragma once

namespace fbpcf::edit_distance {

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    const StringType& data) {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    const StringType& data,
    int partyId) {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    ExtractedAsciiString&& extractedAsciiString) {
  static_assert(isSecret, "Can only recover shared secrets");
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, false, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::openToParty(
    int partyId) const {
  static_assert(isSecret, "No need to open a public value");
  AsciiString<maxWidth, false, schedulerId, usingBatch> rst;
  for (size_t i = 0; i < maxWidth; i++) {
    rst.data_[i] = data_.at(i).openToParty(partyId);
  }
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
typename AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::StringType
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::getValue() const {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
typename AsciiString<maxWidth, true, schedulerId, usingBatch>::
    ExtractedAsciiString
    AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::
        extractAsciiStringShare() const {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
template <int sizeWidth>
Int<false, sizeWidth, true, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::privateSize() const {
  static_assert(isSecret, "Only private values have an unknown size");
  static_assert(
      maxWidth >> (sizeWidth - 1) == 0,
      "Private int width must be large enough for string width");
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::toUpperCase() const {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::toLowerCase() const {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther, int otherMaxWidth>
AsciiString<
    maxWidth + otherMaxWidth,
    isSecret || isSecretOther,
    schedulerId,
    usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::concat(
    const AsciiString<otherMaxWidth, isSecretOther, schedulerId, usingBatch>&
        other) const {
  throw std::runtime_error("Unimplemented");
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretChoice, bool isSecretOther>
AsciiString<
    maxWidth,
    isSecret || isSecretChoice || isSecretOther,
    schedulerId,
    usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::mux(
    const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
    const AsciiString<maxWidth, isSecretOther, schedulerId, usingBatch>& other)
    const {
  throw std::runtime_error("Unimplemented");
}

} // namespace fbpcf::edit_distance
