/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/scheduler/IScheduler.h"

namespace fbpcf::scheduler {

/**
 * Base interface for Gates in a boolean and/or arithmetic circuit. It is used
 * to encapsulate operations that will happen at some point in the future.
 */
class IGate {
 public:
  struct Secrets {
    std::vector<bool> booleanSecrets;
    std::vector<uint64_t> integerSecrets;
    Secrets(
        std::vector<bool> booleanSecrets,
        std::vector<uint64_t> integerSecrets)
        : booleanSecrets(booleanSecrets), integerSecrets(integerSecrets) {}
  };

  virtual ~IGate() = default;

  /* Run or schedule the computation for this gate.
   * @param engine The secret share engine used to execute and schedule
   * networked operations
   * @param secretSharesByParty Will be modified to include any output shares to
   * be revealed for each party ID
   */
  virtual void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, Secrets>& secretSharesByParty) = 0;

  /* For non-free gates, get the result of the computation that was scheduled
   * and store it on the appropriate wire(s).
   * @param engine Will be used to retrieve AND/MULT gates results in XOR/ADD ss
   * form
   * @param revealedSecretsByParty Will hold values for output gates shared from
   * other parties.
   */
  virtual void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, Secrets>& revealedSecretsByParty) = 0;

  // The number of values in a batch gate (1 for non-batch case)
  virtual uint32_t getNumberOfResults() const = 0;
};
} // namespace fbpcf::scheduler
