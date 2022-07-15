/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>
#include "fbpcf/frontend/Int.h"

namespace fbpcf::edit_distance {

/*
 * An MPC representation of a string of Ascii characters. The length of the
 * string is not known to the other party. maxWidth will control the maximum
 * size of the string, and is agreed on ahead of time by the parties. Any unused
 * characters will be represented as \x00. Each character is represented as a
 * signed 8-bit integer and can be retrieved using regular index operations.
 */
template <int maxWidth, bool isSecret, int schedulerId, bool usingBatch = false>
class AsciiString : public scheduler::SchedulerKeeper<schedulerId> {
  static_assert(maxWidth > 1, "Max width must be at least one character");
  using StringType = typename std::
      conditional<usingBatch, std::vector<std::string>, std::string>::type;
  using SizeType =
      typename std::conditional<usingBatch, std::vector<size_t>, size_t>::type;
  using AsciiChar = frontend::Int<true, 8, isSecret, schedulerId, usingBatch>;

 public:
  class ExtractedAsciiString {
   public:
    ExtractedAsciiString() = default;

    explicit ExtractedAsciiString(StringType& data) {
      if constexpr (usingBatch) {
        size_t batchSize = data.size();
        for (size_t j = 0; j < batchSize; j++) {
          if (data[j].size() > maxWidth) {
            throw std::runtime_error(
                "Input value is too large. Maximum string width is %d, was  given %d",
                maxWidth,
                data[j].size());
          }
        }
        for (size_t i = 0; i < maxWidth; i++) {
          std::vector<char> chars(batchSize);
          for (size_t j = 0; j < batchSize; j++) {
            chars[j] = data[i][j];
          }
          data_[i] = frontend::Int<true, 8, true, schedulerId, usingBatch>::
              ExtractedInt(chars);
        }

      } else {
        if (data.size() > maxWidth) {
          throw std::runtime_error(
              "Input value is too large. Maximum string width is %d, was  given %d",
              maxWidth,
              data.size());
        }
        for (size_t i = 0; i < data.size(); i++) {
          data_[i] = frontend::Int<true, 8, true, schedulerId, usingBatch>::
              ExtractedInt(data[i]);
        }

        for (size_t i = data.size(); i < maxWidth + 1; i++) {
          data_[i] = frontend::Int<true, 8, true, schedulerId, usingBatch>::
              ExtractedInt(0);
        }
      }
    }

    typename frontend::Int<true, 8, isSecret, schedulerId, usingBatch>::
        ExtractedInt&
        operator[](int index) {
      return data_[index];
    }

    size_t width() const {
      return maxWidth;
    }

   private:
    std::array<
        typename frontend::Int<true, 8, true, schedulerId, usingBatch>::
            ExtractedInt,
        maxWidth>
        data_;
  };

  // Create uninitialized AsciiString
  AsciiString() : data_() {}

  // Create a public AsciiString
  explicit AsciiString(const StringType& data);

  // Create a private AsciiString from one parties string. Other's input is
  // ignored.
  explicit AsciiString(const StringType& data, int partyId);

  explicit AsciiString(ExtractedAsciiString&& extractedAsciiString);

  // Create a new AsciiString carrying the plaintext value of this string.
  // Only one party will receive the true value, rest will receive dummy values.
  AsciiString<maxWidth, false, schedulerId, usingBatch> openToParty(
      int partyId) const;

  // Get the plaintext value associated with this string
  StringType getValue() const;

  typename AsciiString<maxWidth, true, schedulerId, usingBatch>::
      ExtractedAsciiString
      extractAsciiStringShare() const;

  // returns the length of the string object. Must be a public or revealed value
  SizeType size() const {
    return knownSize_;
  }

  size_t getBatchSize() const {
    static_assert(usingBatch, "Only batch types have batch size");
    return batchSize_;
  }

  // returns the length of the string as a secret integer. Template specifies
  // the width of the Int and must be enough to store the width.
  template <int sizeWidth>
  frontend::Int<false, sizeWidth, true, schedulerId, usingBatch> privateSize()
      const;

  size_t width() const {
    return maxWidth;
  }

  const frontend::Int<true, 8, isSecret, schedulerId, usingBatch>& at(
      size_t i) const {
    return data_.at(i);
  }

  frontend::Int<true, 8, isSecret, schedulerId, usingBatch>& operator[](
      size_t i) {
    return data_[i];
  }

  const frontend::Int<true, 8, isSecret, schedulerId, usingBatch>& operator[](
      size_t i) const {
    return data_[i];
  }

  AsciiString<maxWidth, isSecret, schedulerId, usingBatch> toUpperCase() const;
  AsciiString<maxWidth, isSecret, schedulerId, usingBatch> toLowerCase() const;

  template <bool isSecretOther, int otherMaxWidth>
  AsciiString<
      maxWidth + otherMaxWidth,
      isSecret || isSecretOther,
      schedulerId,
      usingBatch>
  concat(
      const AsciiString<otherMaxWidth, isSecretOther, schedulerId, usingBatch>&
          other) const;

  template <bool isSecretChoice, bool isSecretOther>
  AsciiString<
      maxWidth,
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
  mux(const frontend::Bit<isSecretChoice, schedulerId, usingBatch>& choice,
      const AsciiString<maxWidth, isSecretOther, schedulerId, usingBatch>&
          other) const;

 private:
  std::
      array<frontend::Int<true, 8, isSecret, schedulerId, usingBatch>, maxWidth>
          data_;
  SizeType knownSize_;
  size_t batchSize_ = 1;

  std::vector<char> convertLongsToChar(std::vector<int64_t> values) const;

  friend class AsciiString<maxWidth, !isSecret, schedulerId, usingBatch>;
};

} // namespace fbpcf::edit_distance

#include "./AsciiString_impl.h" // @manual
