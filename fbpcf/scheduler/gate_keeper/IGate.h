/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include "fbpcf/engine/ISecretShareEngine.h"

namespace fbpcf::scheduler {

/**
 * Base interface for Gates in a boolean circuit. It is used to encapsulate
 * operations that will happen at some point in the future.
 */
class IGate {
 public:
  virtual ~IGate() = default;

  /* Run or schedule the computation for this gate.
   * @param engine The secret share engine used to execute and schedule
   * networked operations
   * @param secretSharesByParty Will be modified to include any output shares to
   * be revealed for each party ID
   */
  virtual void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, std::vector<bool>>& secretSharesByParty) = 0;

  /* For non-free gates, get the result of the computation that was scheduled
   * and store it on the appropriate wire(s).
   * @param engine Will be used to retrieve AND gates results in XOR ss form
   * @param revealedSecretsByParty Will hold values for output gates shared from
   * other parties.
   */
  virtual void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, std::vector<bool>>& revealedSecretsByParty) = 0;

  // The number of values in a batch gate (1 for non-batch case)
  virtual uint32_t getNumberOfResults() const = 0;

 protected:
};
} // namespace fbpcf::scheduler
