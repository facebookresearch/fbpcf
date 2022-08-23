/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/format.h>
#include <stdexcept>
#include "./AsciiString.h" // @manual
#include "./Util.h" // @manual

#pragma once

namespace fbpcf::edit_distance {

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    const StringType& data) {
  static_assert(!isSecret, "Private value must not be created public input");
  if constexpr (usingBatch) {
    auto batchSize = data.size();
    knownSize_.reserve(batchSize);
    for (size_t i = 0; i < batchSize; i++) {
      if (data[i].size() > maxWidth) {
        throw std::runtime_error(fmt::format(
            "Input value is too large. Maximum string width is %d, was  given %d",
            maxWidth,
            data[i].size()));
      }
      knownSize_.push_back(data[i].size());
    }
    for (size_t i = 0; i < maxWidth; i++) {
      std::vector<char> chars(batchSize);
      for (size_t j = 0; j < batchSize; j++) {
        chars[j] = i < data[j].size() ? data[j][i] : 0;
      }
      data_[i] = frontend::Int<true, 8, false, schedulerId, usingBatch>(chars);
    }

  } else {
    if (data.size() > maxWidth) {
      throw std::runtime_error(fmt::format(
          "Input value is too large. Maximum string width is %d, was  given %d",
          maxWidth,
          data.size()));
    }
    knownSize_ = data.size();
    for (size_t i = 0; i < data.size(); i++) {
      data_[i] =
          frontend::Int<true, 8, false, schedulerId, usingBatch>(data[i]);
    }

    for (size_t i = data.size(); i < maxWidth; i++) {
      data_[i] = frontend::Int<true, 8, false, schedulerId, usingBatch>(0);
    }
  }
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    const StringType& data,
    int partyId) {
  static_assert(isSecret, "Public value must not be created secret input");
  if constexpr (usingBatch) {
    auto batchSize = data.size();
    for (size_t j = 0; j < batchSize; j++) {
      if (data[j].size() > maxWidth) {
        throw std::runtime_error(fmt::format(
            "Input value is too large. Maximum string width is %d, was  given %d",
            maxWidth,
            data[j].size()));
      }
    }
    for (size_t i = 0; i < maxWidth; i++) {
      std::vector<char> chars(batchSize);
      for (size_t j = 0; j < batchSize; j++) {
        chars[j] = i < data[j].size() ? data[j][i] : 0;
      }
      data_[i] =
          frontend::Int<true, 8, true, schedulerId, usingBatch>(chars, partyId);
    }

  } else {
    if (data.size() > maxWidth) {
      throw std::runtime_error(fmt::format(
          "Input value is too large. Maximum string width is %d, was  given %d",
          maxWidth,
          data.size()));
    }
    for (size_t i = 0; i < data.size(); i++) {
      data_[i] = frontend::Int<true, 8, true, schedulerId, usingBatch>(
          data[i], partyId);
    }

    for (size_t i = data.size(); i < maxWidth; i++) {
      data_[i] =
          frontend::Int<true, 8, true, schedulerId, usingBatch>(0, partyId);
    }
  }
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::AsciiString(
    ExtractedAsciiString&& extractedAsciiString) {
  static_assert(isSecret, "Can only recover shared secrets");
  for (size_t i = 0; i < maxWidth; i++) {
    data_[i] = frontend::Int<true, 8, isSecret, schedulerId, usingBatch>(
        std::move(extractedAsciiString[i]));
  }
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

  if constexpr (usingBatch) {
    std::array<std::vector<char>, maxWidth> values;
    for (int i = 0; i < maxWidth; i++) {
      values[i] = convertLongsToChar(rst.data_[i].getValue());
    }

    size_t batchSize = values[0].size();
    SizeType sizes(batchSize);
    for (int wordIndex = 0; wordIndex < batchSize; wordIndex++) {
      int letterIndex = 0;
      while (letterIndex < maxWidth && values[letterIndex][wordIndex] != 0) {
        letterIndex++;
      }
      sizes[wordIndex] = letterIndex;
    }

    rst.knownSize_ = sizes;

  } else {
    int letterIndex = 0;
    while (letterIndex < maxWidth && rst.data_[letterIndex].getValue() != 0) {
      letterIndex++;
    }
    rst.knownSize_ = letterIndex;
  }

  return rst;
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
typename AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::StringType
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::getValue() const {
  static_assert(!isSecret, "Can't get value on secret wires");
  if constexpr (usingBatch) {
    std::vector<char> chars = convertLongsToChar(data_[0].getValue());
    size_t batchSize = chars.size();
    std::vector<std::string> rst(batchSize);
    for (size_t i = 0; i < batchSize; i++) {
      rst[i].reserve(maxWidth);
      if (chars[i] != 0) {
        rst[i].push_back(chars[i]);
      }
    }
    for (size_t j = 1; j < maxWidth; j++) {
      chars = convertLongsToChar(data_[j].getValue());
      for (size_t i = 0; i < batchSize; i++) {
        if (chars[i] != 0) {
          rst[i].push_back(chars[i]);
        }
      }
    }
    return rst;
  } else {
    std::string rst = "";
    for (size_t i = 0; i < maxWidth; i++) {
      char c = data_[i].getValue();
      if (c != 0) {
        rst.push_back(c);
      }
    }

    return rst;
  }
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
typename AsciiString<maxWidth, true, schedulerId, usingBatch>::
    ExtractedAsciiString
    AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::
        extractAsciiStringShare() const {
  static_assert(isSecret, "No need to extract a public value.");
  ExtractedAsciiString rst;
  for (size_t i = 0; i < maxWidth; i++) {
    rst[i] = data_.at(i).extractIntShare();
  }
  return rst;
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
template <int sizeWidth>
frontend::Int<false, sizeWidth, true, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::privateSize() const {
  static_assert(isSecret, "Only private values have an unknown size");
  static_assert(
      maxWidth >> (sizeWidth - 1) == 0,
      "Private int width must be large enough for string width");
  static_assert(maxWidth > 0, "String can not be always empty");

  frontend::Int<false, sizeWidth, true, schedulerId, usingBatch> rst;
  if constexpr (usingBatch) {
    frontend::Int<false, sizeWidth, false, schedulerId, usingBatch> zeroIndex =
        createPublicBatchConstant<
            frontend::Int<false, sizeWidth, false, schedulerId, usingBatch>,
            uint64_t>(0, getBatchSize());
    frontend::Int<false, sizeWidth, false, schedulerId, usingBatch> one =
        createPublicBatchConstant<
            frontend::Int<false, sizeWidth, false, schedulerId, usingBatch>,
            uint64_t>(1, getBatchSize());
    frontend::Int<true, 8, false, schedulerId, usingBatch> zeroChar =
        createPublicBatchConstant<
            frontend::Int<true, 8, false, schedulerId, usingBatch>,
            int64_t>(0, getBatchSize());
    rst = one.mux(data_[0] == zeroChar, zeroIndex);
    for (size_t i = 1; i < maxWidth; i++) {
      rst = (rst + one).mux(data_[i] == zeroChar, rst);
    }
  } else {
    frontend::Int<false, sizeWidth, false, schedulerId, usingBatch> zeroIndex(
        0U);
    frontend::Int<false, sizeWidth, false, schedulerId, usingBatch> one(1U);
    frontend::Int<true, 8, false, schedulerId, usingBatch> zeroChar(0);
    rst = one.mux(data_[0] == zeroChar, zeroIndex);
    for (size_t i = 1; i < maxWidth; i++) {
      rst = (rst + one).mux(data_[i] == zeroChar, rst);
    }
  }

  return rst;
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::toUpperCase() const {
  if constexpr (usingBatch) {
    AsciiString<maxWidth, isSecret, schedulerId, usingBatch> rst;
    std::vector<int64_t> v1(getBatchSize(), 'a');
    std::vector<int64_t> v2(getBatchSize(), 'z');
    std::vector<int64_t> v3(getBatchSize(), 0);
    std::vector<int64_t> v4(getBatchSize(), 'A' - 'a');
    frontend::Int<true, 8, false, schedulerId, usingBatch> a(v1);
    frontend::Int<true, 8, false, schedulerId, usingBatch> z(v2);
    frontend::Int<true, 8, false, schedulerId, usingBatch> zero(v3);
    frontend::Int<true, 8, false, schedulerId, usingBatch> upper(v4);
    for (int i = 0; i < maxWidth; i++) {
      auto isLower = data_[i] >= a & data_[i] <= z;
      rst.data_[i] = data_[i] + zero.mux(isLower, upper);
    }
    return rst;
  } else {
    AsciiString<maxWidth, isSecret, schedulerId, usingBatch> rst;

    frontend::Int<true, 8, false, schedulerId, usingBatch> a('a');
    frontend::Int<true, 8, false, schedulerId, usingBatch> z('z');
    frontend::Int<true, 8, false, schedulerId, usingBatch> zero(0);
    frontend::Int<true, 8, false, schedulerId, usingBatch> upper('A' - 'a');

    for (size_t i = 0; i < maxWidth; i++) {
      auto isLower = data_[i] >= a & data_[i] <= z;
      rst.data_[i] = data_[i] + zero.mux(isLower, upper);
    }
    return rst;
  }
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
    const frontend::Bit<isSecretChoice, schedulerId, usingBatch>& choice,
    const AsciiString<maxWidth, isSecretOther, schedulerId, usingBatch>& other)
    const {
  AsciiString<
      maxWidth,
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
      rst;

  for (size_t i = 0; i < maxWidth; i++) {
    rst.data_[i] = data_[i].mux(choice, other.data_[i]);
  }

  return rst;
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
std::vector<char>
AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::convertLongsToChar(
    std::vector<int64_t> values) {
  std::vector<char> rst(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    rst[i] = values[i];
  }
  return rst;
}

template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch>
size_t AsciiString<maxWidth, isSecret, schedulerId, usingBatch>::getBatchSize()
    const {
  return data_[0].getBatchSize();
}

} // namespace fbpcf::edit_distance
