/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "fbpcf/mpc_std_lib/unified_data_process/serialization/FixedSizeArrayColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IRowStructureDefinition.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IntegerColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/PackedBitFieldColumn.h"

#include "folly/Format.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
class RowStructureDefinition : public IRowStructureDefinition<schedulerId> {
 public:
  using SecString = frontend::BitString<true, schedulerId, true>;
  using SecBit = frontend::Bit<true, schedulerId, true>;

  explicit RowStructureDefinition(
      std::unique_ptr<
          std::vector<std::unique_ptr<IColumnDefinition<schedulerId>>>>
          columnDefinitions)
      : columnDefinitions_(std::move(columnDefinitions)) {}

  /* Returns the number of bytes to serialize a single row */
  size_t getRowSizeBytes() const override {
    size_t rst = 0;
    for (const auto& columnType : *columnDefinitions_.get()) {
      rst += columnType->getColumnSizeBytes();
    }

    return rst;
  }

  std::vector<std::vector<unsigned char>> serializeDataAsBytesForUDP(
      const std::unordered_map<
          std::string,
          typename IRowStructureDefinition<schedulerId>::InputColumnDataType>&
          data,
      int numRows) const override {
    // validate number of columns matches what is expected
    size_t expectedColumns = 0;
    for (const std::unique_ptr<IColumnDefinition<schedulerId>>&
             columnDefinition : *columnDefinitions_.get()) {
      const PackedBitFieldColumn<schedulerId>* packedBitCol =
          dynamic_cast<const PackedBitFieldColumn<schedulerId>*>(
              columnDefinition.get());

      if (packedBitCol == nullptr) {
        expectedColumns++;
      } else {
        expectedColumns += packedBitCol->getSubColumnNames().size();
      }
    }
    if (data.size() != expectedColumns) {
      throw std::runtime_error(
          "Mismatch between number of columns defined by row structure and what was passed in.");
    }

    size_t byteOffset = 0;

    std::vector<std::vector<unsigned char>> writeBuffers(
        numRows, std::vector<unsigned char>(getRowSizeBytes()));

    for (const std::unique_ptr<IColumnDefinition<schedulerId>>&
             columnDefinition : *columnDefinitions_.get()) {
      const IColumnDefinition<schedulerId>* columnPointer =
          columnDefinition.get();
      auto columnType = columnDefinition->getColumnType();

      switch (columnType) {
        case IColumnDefinition<
            schedulerId>::SupportedColumnTypes::PackedBitField:
          serializePackedBitFieldColumn(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32:
          serializeIntegerColumn<false, 32>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32:
          serializeIntegerColumn<true, 32>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64:
          serializeIntegerColumn<true, 64>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32Vec:
          serializeVectorColumn<
              std::vector<uint32_t>,
              typename frontend::MPCTypes<schedulerId>::SecUnsigned32Int>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32Vec:
          serializeVectorColumn<
              std::vector<int32_t>,
              typename frontend::MPCTypes<schedulerId>::Sec32Int>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64Vec:
          serializeVectorColumn<
              std::vector<int64_t>,
              typename frontend::MPCTypes<schedulerId>::Sec64Int>(
              columnPointer, data, writeBuffers, numRows, byteOffset);
          break;
        default:
          throw std::runtime_error(
              "Unknown column type while serializing data.");
      }

      byteOffset += columnPointer->getColumnSizeBytes();
    }

    return writeBuffers;
  }

  // Following a run of the UDP protocol, deserialize the array into pointers
  // to MPC types. Data is represented in column order format
  virtual std::unordered_map<
      std::string,
      typename IColumnDefinition<schedulerId>::DeserializeType>
  deserializeUDPOutputIntoMPCTypes(
      const SecString& secretSharedData) const override {
    std::vector<std::vector<bool>> secretSharedBits =
        secretSharedData.extractStringShare().getValue();
    secretSharedBits = transpose(secretSharedBits);
    std::vector<std::vector<unsigned char>> secretSharedBytes(0);
    secretSharedBytes.reserve(secretSharedBits.size());
    for (int i = 0; i < secretSharedBits.size(); i++) {
      secretSharedBytes.push_back(convertFromBits(secretSharedBits[i]));
    }

    std::unordered_map<
        std::string,
        typename IColumnDefinition<schedulerId>::DeserializeType>
        rst;
    size_t byteOffset = 0;
    for (const std::unique_ptr<IColumnDefinition<schedulerId>>&
             columnDefinition : *columnDefinitions_.get()) {
      rst.emplace(
          columnDefinition->getColumnName(),
          columnDefinition->deserializeSharesToMPCType(
              secretSharedBytes, byteOffset));
      byteOffset += columnDefinition->getColumnSizeBytes();
    }

    return rst;
  }

 private:
  // use an ordered map for consistency between both parties
  std::unique_ptr<std::vector<std::unique_ptr<IColumnDefinition<schedulerId>>>>
      columnDefinitions_;

  std::vector<unsigned char> convertFromBits(
      const std::vector<bool>& data) const {
    std::vector<unsigned char> rst;
    rst.reserve(data.size() / 8);

    size_t i = 0;

    while (i < data.size()) {
      unsigned char val = 0;
      size_t bitsLeft = data.size() - i > 8 ? 8 : data.size() - i;
      for (auto j = 0; j < bitsLeft; j++) {
        val |= (data[i] << j);
        ++i;
      }
      rst.push_back(val);
    }

    return rst;
  }

  template <typename T>
  std::vector<std::vector<T>> transpose(
      const std::vector<std::vector<T>>& data) const {
    std::vector<std::vector<T>> result;
    if (data.size() == 0) {
      return result;
    }

    result.reserve(data[0].size());
    for (size_t column = 0; column < data[0].size(); column++) {
      std::vector<T> innerArray(data.size());
      result.push_back(std::vector<T>(data.size()));
      for (size_t row = 0; row < data.size(); row++) {
        result[column][row] = data[row][column];
      }
    }
    return result;
  }

  void serializePackedBitFieldColumn(
      const IColumnDefinition<schedulerId>* columnPointer,
      const std::unordered_map<
          std::string,
          typename IRowStructureDefinition<schedulerId>::InputColumnDataType>&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      int numRows,
      size_t byteOffset) const {
    const PackedBitFieldColumn<schedulerId>* packedBitCol =
        dynamic_cast<const PackedBitFieldColumn<schedulerId>*>(columnPointer);

    if (packedBitCol == nullptr) {
      throw std::runtime_error("Failed to cast to PackedBitFieldColumn");
    }
    std::vector<std::vector<bool>> bitPack(
        numRows, std::vector<bool>(packedBitCol->getSubColumnNames().size()));

    for (int i = 0; i < packedBitCol->getSubColumnNames().size(); i++) {
      std::string colName = packedBitCol->getSubColumnNames()[i];
      if (!inputData.contains(colName)) {
        throw std::runtime_error(
            "Column: " + colName +
            " which was defined in the structure was not included in the input data map.");
      }

      const std::vector<bool> bitVals =
          std::get<std::vector<bool>>(inputData.at(colName));

      if (bitVals.size() != numRows) {
        std::string err = folly::sformat(
            "Invalid number of values for column {} .Got {} values but number of rows should be {} ",
            colName,
            bitVals.size(),
            numRows);
        throw std::runtime_error(err);
      }

      for (int j = 0; j < numRows; j++) {
        bitPack[j][i] = bitVals[j];
      }
    }

    for (int i = 0; i < numRows; i++) {
      packedBitCol->serializeColumnAsPlaintextBytes(
          bitPack.data() + i, writeBuffers[i].data() + byteOffset);
    }
  }

  template <bool isSigned, int8_t width>
  void serializeIntegerColumn(
      const IColumnDefinition<schedulerId>* columnPointer,
      const std::unordered_map<
          std::string,
          typename IRowStructureDefinition<schedulerId>::InputColumnDataType>&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      int numRows,
      size_t byteOffset) const {
    std::string colName = columnPointer->getColumnName();

    if (!inputData.contains(colName)) {
      throw std::runtime_error(folly::sformat(
          "Column {} which was defined in the structure was not included"
          " in the input data map.",
          colName));
    }

    using IntType =
        typename IntegerColumn<schedulerId, isSigned, width>::NativeType;

    const std::vector<IntType> intVals =
        std::get<std::vector<IntType>>(inputData.at(colName));

    if (intVals.size() != numRows) {
      std::string err = folly::sformat(
          "Invalid number of values for column {}. Got {} values but number of rows should be {} ",
          colName,
          intVals.size(),
          numRows);
      throw std::runtime_error(err);
    }

    for (int i = 0; i < numRows; i++) {
      columnPointer->serializeColumnAsPlaintextBytes(
          &intVals[i], writeBuffers[i].data() + byteOffset);
    }
  }

  template <typename DataType, typename InnerColumnType>
  void serializeVectorColumn(
      const IColumnDefinition<schedulerId>* columnPointer,
      const std::unordered_map<
          std::string,
          typename IRowStructureDefinition<schedulerId>::InputColumnDataType>&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      int numRows,
      size_t byteOffset) const {
    const FixedSizeArrayColumn<schedulerId, InnerColumnType>* vectorColumn =
        dynamic_cast<const FixedSizeArrayColumn<schedulerId, InnerColumnType>*>(
            columnPointer);
    std::string colName = columnPointer->getColumnName();

    if (vectorColumn == nullptr) {
      throw std::runtime_error("Failed to cast to vector column type");
    }

    if (!inputData.contains(colName)) {
      throw std::runtime_error(folly::sformat(
          "Column {} which was defined in the structure was not included"
          " in the input data map.",
          vectorColumn->getColumnName()));
    }

    const std::vector<DataType> vectorVals =
        std::get<std::vector<DataType>>(inputData.at(colName));

    if (vectorVals.size() != numRows) {
      std::string err = folly::sformat(
          "Invalid number of values for column {}. Got {} values but number of rows should be {}",
          colName,
          vectorVals.size(),
          numRows);
      throw std::runtime_error(err);
    }

    for (int i = 0; i < numRows; i++) {
      if (vectorVals[i].size() != vectorColumn->getLength()) {
        std::string err = folly::sformat(
            "Invalid number of values in array at index {}. Got {} values but number of rows should be {}",
            i,
            vectorVals[i].size(),
            vectorColumn->getLength());
        throw std::runtime_error(err);
      }

      vectorColumn->serializeColumnAsPlaintextBytes(
          vectorVals[i].data(), writeBuffers[i].data() + byteOffset);
    }
  }
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
