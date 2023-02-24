/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/frontend/MPCTypes.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IColumnDefinition.h"

#include <string>
#include "folly/Format.h"
namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
class PackedBitFieldColumn : public IColumnDefinition<schedulerId> {
  using NativeType = std::vector<bool>;
  using MPCTypes = frontend::MPCTypes<schedulerId, true>;

 public:
  PackedBitFieldColumn(
      std::string columnName,
      std::vector<std::string> subColumnNames)
      : columnName_{columnName}, subColumnNames_{subColumnNames} {
    if (subColumnNames.size() > 8) {
      throw std::runtime_error(
          "Can only pack 8 bits into a byte. Please create another PackedBitField"
          " if you would like to store additional boolean values");
    }
  }

  std::string getColumnName() const override {
    return columnName_;
  }

  const std::vector<std::string>& getSubColumnNames() const {
    return subColumnNames_;
  }

  size_t getColumnSizeBytes() const override {
    return 1;
  }

  typename IColumnDefinition<schedulerId>::SupportedColumnTypes getColumnType()
      const override {
    return IColumnDefinition<schedulerId>::SupportedColumnTypes::PackedBitField;
  }

  // input to this function is a pointer to a bool vector since memory layout
  // is not guaranteed by compiler (i.e. can not get a bool* from a
  // vector<bool>.data())
  void serializeColumnAsPlaintextBytes(
      const typename IColumnDefinition<schedulerId>::InputColumnDataType&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      size_t byteOffset) const override {
    const std::vector<NativeType>& bitPack =
        std::get<std::reference_wrapper<std::vector<NativeType>>>(inputData)
            .get();

    for (size_t i = 0; i < writeBuffers.size(); i++) {
      unsigned char toWrite = 0;
      for (size_t j = 0; j < bitPack[i].size(); j++) {
        toWrite |= (bitPack[i][j] << j);
      }
      writeBuffers[i][byteOffset] = toWrite;
    }
  }

  typename IColumnDefinition<schedulerId>::DeserializeType
  deserializeSharesToMPCType(
      const std::vector<std::vector<unsigned char>>& serializedSecretShares,
      size_t offset) const override {
    std::vector<std::vector<bool>> reconstructedShares(
        subColumnNames_.size(),
        std::vector<bool>(serializedSecretShares.size()));

    for (int i = 0; i < serializedSecretShares.size(); i++) {
      unsigned char packedValues = serializedSecretShares[i][offset];
      for (int j = 0; j < subColumnNames_.size(); j++) {
        reconstructedShares[j][i] = (packedValues >> j) & 1;
      }
    }

    std::vector<typename MPCTypes::SecBool> rst(reconstructedShares.size());
    for (int i = 0; i < reconstructedShares.size(); i++) {
      rst[i] = typename MPCTypes::SecBool(
          typename MPCTypes::SecBool::ExtractedBit(reconstructedShares[i]));
    }

    return rst;
  }

 private:
  std::string columnName_;
  std::vector<std::string> subColumnNames_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
