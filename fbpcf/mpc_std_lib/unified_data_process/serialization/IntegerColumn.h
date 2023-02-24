/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/frontend/Int.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IColumnDefinition.h"

#include <stdexcept>
#include <string>

#include "folly/Format.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId, bool isSigned, int8_t width>
class IntegerColumn : public IColumnDefinition<schedulerId> {
  static_assert(
      width == 8 || width == 16 || width == 32 || width == 64,
      "Can only serialize on standard integer types");

  template <
      typename T8,
      typename T16,
      typename T32,
      typename T64,
      int8_t intWidth>
  using typeSelector = typename std::conditional<
      intWidth <= 16,
      typename std::conditional<intWidth <= 8, T8, T16>::type,
      typename std::conditional<intWidth <= 32, T32, T64>::type>::type;

 public:
  using NativeType = typename std::conditional<
      isSigned,
      typeSelector<int8_t, int16_t, int32_t, int64_t, width>,
      typeSelector<uint8_t, uint16_t, uint32_t, uint64_t, width>>::type;

  using ShareType =
      typename std::conditional<isSigned, int64_t, uint64_t>::type;

  using MPCNativeType = frontend::Int<isSigned, width, true, schedulerId, true>;

  explicit IntegerColumn(std::string columnName) : columnName_{columnName} {}

  std::string getColumnName() const override {
    return columnName_;
  }

  size_t getColumnSizeBytes() const override {
    return width / 8;
  }

  typename IColumnDefinition<schedulerId>::SupportedColumnTypes getColumnType()
      const override {
    static_assert(
        (isSigned && (width == 32 || width == 64)) || width == 32,
        "For now only support int32, int64, uint64");
    if constexpr (isSigned) {
      if constexpr (width == 32) {
        return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32;
      } else if constexpr (width == 64) {
        return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64;
      }
    } else if constexpr (width == 32) {
      return IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32;
    }
    throw std::runtime_error(folly::sformat(
        "This code should be unreachable. {}int{}_t column type is not supported",
        isSigned ? "" : "u",
        width));
  }

  void serializeColumnAsPlaintextBytes(
      const typename IColumnDefinition<schedulerId>::InputColumnDataType&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      size_t byteOffset) const override {
    const std::vector<NativeType>& intVals =
        std::get<std::reference_wrapper<std::vector<NativeType>>>(inputData)
            .get();

    if (intVals.size() != writeBuffers.size()) {
      std::string err = folly::sformat(
          "Invalid number of values for column {}. Got {} values but number of rows should be {} ",
          columnName_,
          intVals.size(),
          writeBuffers.size());
      throw std::runtime_error(err);
    }

    for (size_t i = 0; i < writeBuffers.size(); i++) {
      for (size_t j = 0; j < sizeof(NativeType); j++) {
        writeBuffers[i][j + byteOffset] = extractByte(intVals[i], j);
      }
    }
  }

  typename IColumnDefinition<schedulerId>::DeserializeType
  deserializeSharesToMPCType(
      const std::vector<std::vector<unsigned char>>& serializedSecretShares,
      size_t byteOffset) const override {
    std::vector<ShareType> reconstructedShares(serializedSecretShares.size());

    for (int i = 0; i < serializedSecretShares.size(); i++) {
      reconstructedShares[i] = reconstructFromBytes<NativeType>(
          serializedSecretShares[i].data() + byteOffset);
    }

    typename IColumnDefinition<schedulerId>::DeserializeType rst =
        MPCNativeType(
            typename MPCNativeType::ExtractedInt(reconstructedShares));

    return typename IColumnDefinition<schedulerId>::DeserializeType(
        std::move(rst));
  }

 private:
  template <typename T>
  static unsigned char extractByte(T val, size_t byte) {
    if (byte < 0 || byte >= sizeof(T)) {
      throw std::invalid_argument("Not enough bytes in type");
    }

    return (uint8_t)(val >> 8 * byte);
  }

  template <typename T>
  static T reconstructFromBytes(const unsigned char* data) {
    T val = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      val |= ((T) * (data + i)) << (i * 8);
    }
    return val;
  }

  std::string columnName_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
