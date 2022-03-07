/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

/*
 * a write-only oram for type T, T must be an addable type and there are
 * corresponding implementation of convertToBits<T>(const T& src);
 * Requirement for type T:
 * It must have a zero value;
 * It must support addition/subtraction/negation/mux and multiplication with a
 * bool. None of these operations should have the potential for undefined
 * behavior.
 */
template <typename T>
class IWriteOnlyOram {
 public:
  // A write-only oram can only support 2 party at this moment. Their role are
  // mostly symmetric.
  enum Role {
    Alice,
    Bob,
  };

  virtual ~IWriteOnlyOram() = default;

  /**
   *  publicly open the secret at position[publicIndex] to a party.
   * @param publicIndex a public index to indicate which secret to reveal, both
   * party should input the same value.
   * @param receiver which party should receive the result
   */
  virtual T publicRead(size_t publicIndex, Role receiver) const = 0;

  /**
   * Reveal the secret at position[publicIndex] to both parties as additive
   * secret shares.
   * @param publicIndex a public index to indicate which secret to reveal, both
   * party should input the same value.
   */
  virtual T secretRead(size_t publicIndex) const = 0;

  /**
   * obliviously add a batch of XOR-secret-shared secrets to a batch of
   * ADDITIVE-secret-shared positions.
   * @param indexShares this party's shares of the indexes, from share batches
   * of less significant to share batches of more significant;
   * @param values this party's shares of values to add, from share batches
   * of less significant to share batches of more significant;
   */
  virtual void obliviousAddBatch(
      const std::vector<std::vector<bool>>& indexShares,
      const std::vector<std::vector<bool>>& values) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
