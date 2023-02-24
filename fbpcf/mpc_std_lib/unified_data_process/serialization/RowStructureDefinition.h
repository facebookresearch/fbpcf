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

#include "IColumnDefinition.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/FixedSizeArrayColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IRowStructureDefinition.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/IntegerColumn.h"
#include "fbpcf/mpc_std_lib/unified_data_process/serialization/PackedBitFieldColumn.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
class RowStructureDefinition : public IRowStructureDefinition<schedulerId> {
 public:
  using SecString = frontend::BitString<true, schedulerId, true>;

  explicit RowStructureDefinition(
      std::map<
          std::string,
          typename IColumnDefinition<schedulerId>::SupportedColumnTypes>
          columnDefinitions,
      size_t paddingSize = 1) {
    columnDefinitions_ = std::make_unique<
        std::vector<std::unique_ptr<IColumnDefinition<schedulerId>>>>(0);

    std::vector<std::string> bitColumns(0);
    size_t bytesPacked = 0;

    for (auto& columnNameToType : columnDefinitions) {
      switch (columnNameToType.second) {
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Bit:
          if (bitColumns.size() == 8) {
            columnDefinitions_->push_back(
                std::make_unique<PackedBitFieldColumn<schedulerId>>(
                    "packedBits" + std::to_string(bytesPacked), bitColumns));
            bitColumns.clear();
            bytesPacked++;
          }
          bitColumns.push_back(columnNameToType.first);
          break;
        case IColumnDefinition<
            schedulerId>::SupportedColumnTypes::PackedBitField:
          throw std::runtime_error(
              "Can not directly instantiate PackedBit field column. Please use Bit column instead");
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32:
          columnDefinitions_->push_back(
              std::make_unique<IntegerColumn<schedulerId, false, 32>>(
                  columnNameToType.first));
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32:
          columnDefinitions_->push_back(
              std::make_unique<IntegerColumn<schedulerId, true, 32>>(
                  columnNameToType.first));
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64:
          columnDefinitions_->push_back(
              std::make_unique<IntegerColumn<schedulerId, true, 64>>(
                  columnNameToType.first));
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::UInt32Vec:
          columnDefinitions_->push_back(
              std::make_unique<FixedSizeArrayColumn<
                  schedulerId,
                  typename frontend::MPCTypes<schedulerId>::SecUnsigned32Int,
                  uint32_t>>(columnNameToType.first, paddingSize));

          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int32Vec:
          columnDefinitions_->push_back(
              std::make_unique<FixedSizeArrayColumn<
                  schedulerId,
                  typename frontend::MPCTypes<schedulerId>::Sec32Int,
                  int32_t>>(columnNameToType.first, paddingSize));
          break;
        case IColumnDefinition<schedulerId>::SupportedColumnTypes::Int64Vec:
          columnDefinitions_->push_back(
              std::make_unique<FixedSizeArrayColumn<
                  schedulerId,
                  typename frontend::MPCTypes<schedulerId>::Sec64Int,
                  int64_t>>(columnNameToType.first, paddingSize));
          break;
        default:
          throw std::runtime_error(
              "Unknown column type while serializing data.");
      }
    }

    if (!bitColumns.empty()) {
      columnDefinitions_->push_back(
          std::make_unique<PackedBitFieldColumn<schedulerId>>(
              "packedBits" + std::to_string(bytesPacked), bitColumns));
    }
  }

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
          typename IColumnDefinition<schedulerId>::InputColumnDataType>& data,
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

      if (columnDefinition->getColumnType() ==
          IColumnDefinition<
              schedulerId>::SupportedColumnTypes::PackedBitField) {
        serializePackedBitFieldColumn(
            columnPointer, data, writeBuffers, byteOffset);
      } else {
        std::string colName = columnPointer->getColumnName();

        if (!data.contains(colName)) {
          throw std::runtime_error(folly::sformat(
              "Column {} which was defined in the structure was not included"
              " in the input data map.",
              columnPointer->getColumnName()));
        }

        columnPointer->serializeColumnAsPlaintextBytes(
            data.at(colName), writeBuffers, byteOffset);
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
      if (columnDefinition->getColumnType() ==
          IColumnDefinition<
              schedulerId>::SupportedColumnTypes::PackedBitField) {
        const PackedBitFieldColumn<schedulerId>* packedBitCol =
            dynamic_cast<const PackedBitFieldColumn<schedulerId>*>(
                columnDefinition.get());
        auto deserializedMPCValues = std::get<std::vector<
            typename frontend::MPCTypes<schedulerId>::MPCTypes::SecBool>>(
            columnDefinition->deserializeSharesToMPCType(
                secretSharedBytes, byteOffset));
        for (int i = 0; i < packedBitCol->getSubColumnNames().size(); i++) {
          rst.emplace(
              packedBitCol->getSubColumnNames()[i], deserializedMPCValues[i]);
        }
      } else {
        rst.emplace(
            columnDefinition->getColumnName(),
            columnDefinition->deserializeSharesToMPCType(
                secretSharedBytes, byteOffset));
      }

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
          typename IColumnDefinition<schedulerId>::InputColumnDataType>&
          inputData,
      std::vector<std::vector<unsigned char>>& writeBuffers,
      size_t byteOffset) const {
    const PackedBitFieldColumn<schedulerId>* packedBitCol =
        dynamic_cast<const PackedBitFieldColumn<schedulerId>*>(columnPointer);

    if (packedBitCol == nullptr) {
      throw std::runtime_error("Failed to cast to PackedBitFieldColumn");
    }
    std::vector<std::vector<bool>> bitPack(
        writeBuffers.size(),
        std::vector<bool>(packedBitCol->getSubColumnNames().size()));

    for (int i = 0; i < packedBitCol->getSubColumnNames().size(); i++) {
      std::string colName = packedBitCol->getSubColumnNames()[i];
      if (!inputData.contains(colName)) {
        throw std::runtime_error(
            "Column: " + colName +
            " which was defined in the structure was not included in the input data map.");
      }

      const std::vector<bool>& bitVals =
          std::get<std::reference_wrapper<std::vector<bool>>>(
              inputData.at(colName))
              .get();

      if (bitVals.size() != writeBuffers.size()) {
        std::string err = folly::sformat(
            "Invalid number of values for column {}. Got {} values but number of rows should be {} ",
            colName,
            bitVals.size(),
            writeBuffers.size());
        throw std::runtime_error(err);
      }

      for (int j = 0; j < writeBuffers.size(); j++) {
        bitPack[j][i] = bitVals[j];
      }
    }

    for (int i = 0; i < writeBuffers.size(); i++) {
      packedBitCol->serializeColumnAsPlaintextBytes(
          bitPack, writeBuffers, byteOffset);
    }
  }
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
