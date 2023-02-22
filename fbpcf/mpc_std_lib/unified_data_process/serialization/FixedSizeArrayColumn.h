/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <stdexcept>
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IColumnDefinition.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId, typename InnerType>
class FixedSizeArrayColumn : public IColumnDefinition<schedulerId> {
 public:
  FixedSizeArrayColumn(
      std::string columnName,
      std::unique_ptr<IColumnDefinition<schedulerId>> innerType,
      size_t length)
      : columnName_{columnName},
        innerType_{std::move(innerType)},
        length_{length} {}

  std::string getColumnName() const override {
    return columnName_;
  }

  size_t getColumnSizeBytes() const override {
    return length_ * innerType_->getColumnSizeBytes();
  }

  typename IColumnDefinition<schedulerId>::SupportedColumnTypes getColumnType()
      const override {
    auto innerColumnType = innerType_->getColumnType();

    switch (innerColumnType) {
      case IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32:
        return IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32Vec;
      case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32:
        return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32Vec;
      case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64:
        return IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64Vec;
      default:
        throw std::runtime_error(
            "This code should be unreachable. Tried to get invalid Array Column");
    }
  }

  size_t getLength() const {
    return length_;
  }

  void serializeColumnAsPlaintextBytes(
      const void* inputData,
      unsigned char* buf) const override {
    size_t innerTypeSize = innerType_->getColumnSizeBytes();
    for (int i = 0; i < length_; i++) {
      size_t offset = innerTypeSize * i;
      innerType_->serializeColumnAsPlaintextBytes(
          (const void*)(((const unsigned char*)inputData) + offset),
          buf + offset);
    }
  }

  typename IColumnDefinition<schedulerId>::DeserializeType
  deserializeSharesToMPCType(
      const std::vector<std::vector<unsigned char>>& serializedSecretShares,
      size_t byteOffset) const override {
    auto rst = std::vector<InnerType>(0);

    size_t innerTypeSize = innerType_->getColumnSizeBytes();

    for (int i = 0; i < length_; i++) {
      size_t offset = byteOffset + innerTypeSize * i;
      typename IColumnDefinition<schedulerId>::DeserializeType innerVal =
          innerType_->deserializeSharesToMPCType(
              serializedSecretShares, offset);
      rst.push_back(std::get<InnerType>(innerVal));
    }

    return rst;
  }

 private:
  std::string columnName_;
  std::unique_ptr<IColumnDefinition<schedulerId>> innerType_;
  size_t length_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
