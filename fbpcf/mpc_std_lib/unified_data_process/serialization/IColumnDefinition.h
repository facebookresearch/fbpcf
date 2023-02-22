/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include "fbpcf/frontend/BitString.h"
#include "fbpcf/frontend/MPCTypes.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
class IColumnDefinition {
  using MPCTypes = frontend::MPCTypes<schedulerId, true /* usingBatch */>;

 public:
  /* Possible return types for deserialization following UDP run */
  using DeserializeType = std::variant<
      typename MPCTypes::SecBool,
      typename MPCTypes::SecUnsigned32Int,
      typename MPCTypes::Sec32Int,
      typename MPCTypes::Sec64Int,
      std::vector<typename MPCTypes::SecBool>,
      std::vector<typename MPCTypes::SecUnsigned32Int>,
      std::vector<typename MPCTypes::Sec32Int>,
      std::vector<typename MPCTypes::Sec64Int>>;

  virtual ~IColumnDefinition() = default;

  virtual std::string getColumnName() const = 0;

  virtual size_t getColumnSizeBytes() const = 0;

  /* Pass in a single value of the column to be serialized, sequentially write
   * the bytes starting at the beginning of buf */
  virtual void serializeColumnAsPlaintextBytes(
      const void* inputData,
      unsigned char* buf) const = 0;

  /* Given the secret shared output of bytes following the UDP stage,
   * load the values into the MPC type correponding to this column.
   * Data starts loading at the offset passed in, and will read the next
   * getColumnSizeBytes() from each row. Caller is responsible for unpacking
   * the variant for the column type.
   */
  virtual DeserializeType deserializeSharesToMPCType(
      const std::vector<std::vector<unsigned char>>& serializedSecretShares,
      size_t offset) const = 0;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
