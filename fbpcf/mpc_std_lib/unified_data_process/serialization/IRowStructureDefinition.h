/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>
#include "IColumnDefinition.h"
#include "fbpcf/frontend/BitString.h"

namespace fbpcf::mpc_std_lib::unified_data_process::serialization {

template <int schedulerId>
class IRowStructureDefinition {
 public:
  using SecString = frontend::BitString<true, schedulerId, true>;

  virtual ~IRowStructureDefinition() = default;

  /* Returns the number of bytes to serialize a single row */
  virtual size_t getRowSizeBytes() const = 0;

  // Serialize each column's worth of data according to the structure
  // definition. Each key must match the name of a column in the definition and
  // the value contains the data for that column
  virtual std::vector<std::vector<unsigned char>> serializeDataAsBytesForUDP(
      const std::unordered_map<
          std::string,
          typename IColumnDefinition<schedulerId>::InputColumnDataType>& data,
      int numRows) const = 0;

  // Following a run of the UDP protocol, deserialize the batched BitString
  // containing encrypted columns into private MPC types.
  virtual std::unordered_map<
      std::string,
      typename IColumnDefinition<schedulerId>::DeserializeType>
  deserializeUDPOutputIntoMPCTypes(const SecString& secretSharedData) const = 0;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::serialization
