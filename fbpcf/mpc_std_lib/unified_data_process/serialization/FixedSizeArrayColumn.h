/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IColumnDefinition.h"

#include "folly/Format.h"
namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId, typename InnerMPCType, typename InnerPlaintextType>
class FixedSizeArrayColumn : public IColumnDefinition<schedulerId> {
  static_assert(
      std::is_same<InnerPlaintextType, uint32_t>::value ||
          std::is_same<InnerPlaintextType, int32_t>::value ||
          std::is_same<InnerPlaintextType, int64_t>::value,
      "Currently only supported types are vec<int32>, vec<uint32_t>, vec<int64>");

  using ShareType = typename std::conditional<
      std::is_same<InnerPlaintextType, uint32_t>::value,
      uint64_t,
      int64_t>::type;

 public:
  FixedSizeArrayColumn(std::string columnName, size_t length)
      : columnName_{columnName}, length_{length} {}

  std::string getColumnName() const override {
    return columnName_;
  }

  size_t getColumnSizeBytes() const override {
    return length_ * sizeof(InnerPlaintextType);
  }

  typename IColumnDefinition<schedulerId>::SupportedColumnTypes getColumnType()
      const override {
    if constexpr (std::is_same<InnerPlaintextType, uint32_t>::value) {
      return IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32Vec;
    } else if constexpr (std::is_same<InnerPlaintextType, int32_t>::value) {
      return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32Vec;
    } else if constexpr (std::is_same<InnerPlaintextType, int64_t>::value) {
      return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64Vec;
    } else {
      throw std::runtime_error(
          "This code should be unreachable. Tried to get invalid Array Column");
    }
  }

  size_t getLength() const {
    return length_;
  }

  void serializeColumnAsPlaintextBytes(
      const typename IColumnDefinition<schedulerId>::InputColumnDataType&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      size_t byteOffset) const override {
    const std::vector<std::vector<InnerPlaintextType>>& vectorVals =
        std::get<std::reference_wrapper<
            std::vector<std::vector<InnerPlaintextType>>>>(inputData)
            .get();

    if (vectorVals.size() != writeBuffers.size()) {
      std::string err = folly::sformat(
          "Invalid number of values for column {}. Got {} values but number of rows should be {} ",
          columnName_,
          vectorVals.size(),
          writeBuffers.size());
      throw std::runtime_error(err);
    }

    for (size_t i = 0; i < writeBuffers.size(); i++) {
      if (vectorVals[i].size() != length_) {
        std::string err = folly::sformat(
            "Invalid number of values in array at index {}. Got {} values but number of rows should be {}",
            i,
            vectorVals[i].size(),
            length_);
        throw std::runtime_error(err);
      }
      for (size_t j = 0; j < length_; j++) {
        for (size_t k = 0; k < sizeof(InnerPlaintextType); k++) {
          writeBuffers[i][byteOffset + j * sizeof(InnerPlaintextType) + +k] =
              extractByte(vectorVals[i][j], k);
        }
      }
    }
  }

  typename IColumnDefinition<schedulerId>::DeserializeType
  deserializeSharesToMPCType(
      const std::vector<std::vector<unsigned char>>& serializedSecretShares,
      size_t byteOffset) const override {
    auto rst = std::vector<InnerMPCType>(0);

    for (int i = 0; i < length_; i++) {
      std::vector<ShareType> reconstructedShares(serializedSecretShares.size());

      for (int j = 0; j < serializedSecretShares.size(); j++) {
        reconstructedShares[j] = reconstructFromBytes<InnerPlaintextType>(
            serializedSecretShares[j].data() + byteOffset +
            sizeof(InnerPlaintextType) * i);
      }

      rst.push_back(InnerMPCType(
          typename InnerMPCType::ExtractedInt(reconstructedShares)));
    }

    return rst;
  }

 private:
  template <typename T>
  unsigned char extractByte(T val, size_t byte) const {
    if (byte < 0 || byte >= sizeof(T)) {
      throw std::invalid_argument("Not enough bytes in type");
    }

    return (uint8_t)(val >> 8 * byte);
  }

  template <typename T>
  T reconstructFromBytes(const unsigned char* data) const {
    T val = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      val |= ((T) * (data + i)) << (i * 8);
    }
    return val;
  }

  std::string columnName_;
  std::unique_ptr<IColumnDefinition<schedulerId>> innerType_;
  size_t length_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
