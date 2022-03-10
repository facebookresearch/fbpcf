/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstdint>
#include <optional>
#include <vector>

namespace fbpcf::engine {

/**
 The MPC engine API
 */
class ISecretShareEngine {
 public:
  virtual ~ISecretShareEngine() = default;

  /**
   * Generate a private input wire carring party id's input
   * @param id the party that own v
   * @param v the plaintext input, this value can be std::nullopt if this
   * party doesn't own v
   * @return the ciphertext form
   */
  virtual bool setInput(int id, std::optional<bool> v = std::nullopt) = 0;

  /**
   * Generate a batch of private input wires carring party id's inputs
   * @param id the party that own v
   * @param v the plaintext inputs, the optional bools in this vector can be
   * any value if this party doesn't own v
   * @return the ciphertext form
   */
  virtual std::vector<bool> setBatchInput(
      int id,
      const std::vector<bool>& v) = 0;

  /**
   * Compute an XOR gate with two private or two public values. This operation
   * requires all parties to XOR their shares/values (That's why it is
   * symmetric). This computation can be done locally, without any interaction
   * with other parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual bool computeSymmetricXOR(bool left, bool right) const = 0;

  /**
   * Compute a batch of XOR gates with two private or two public values. This
   * operation requires all parties to XOR their shares/values (That's why it is
   * symmetric). This computation can be done locally, without any interaction
   * with other parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual std::vector<bool> computeBatchSymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const = 0;

  /**
   * Compute an XOR gate between a private and a public value. This operation
   * requires only one party to XOR his/her share/value (That's why it is
   * asymmetric), others only needs to output left wire value. This computation
   * can be done locally, without any interaction with other parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual bool computeAsymmetricXOR(bool left, bool right) const = 0;

  /**
   * Compute a batch of XOR gates with a private and a public values.  This
   * operation requires only one party to XOR his/her share/value (That's why it
   * is asymmetric), others only needs to output left wire value. This
   * computation can be done locally, without any interaction with other
   * parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual std::vector<bool> computeBatchAsymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const = 0;

  /**
   * Compute a NOT gate on public values, This operation
   * requires all party to flip their shares/values (That's why it is
   * symmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual bool computeSymmetricNOT(bool input) const = 0;

  /**
   * Compute a batch of NOT gate on public values, This operation
   * requires all party to flip their shares/values (That's why it is
   * symmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual std::vector<bool> computeBatchSymmetricNOT(
      const std::vector<bool>& input) const = 0;

  /**
   * Compute a NOT gate on private values, This operation
   * requires only one party to flip his/her share/value (That's why it is
   * asymmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual bool computeAsymmetricNOT(bool input) const = 0;

  /**
   * Compute a batch of NOT gate on private values, This operation
   * requires only one party to flip his/her share/value (That's why it is
   * asymmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual std::vector<bool> computeBatchAsymmetricNOT(
      const std::vector<bool>& input) const = 0;

  /**
   * Compute a free AND gate: at least one of the input is a public value, thus
   * the parties only need to multiply the secret share and the public value
   * (or two public values) locally, without interaction with peers.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual bool computeFreeAND(bool left, bool right) const = 0;

  /**
   * Compute a batch of free AND gate: at least one of the input is a public
   * value, thus the parties only need to multiply the secret share and the
   * public value (or two public values) locally, without interaction with
   * peers.
   * @param left the values on left input wires
   * @param right the values on right input wires
   * @return the values of the results
   */
  virtual std::vector<bool> computeBatchFreeAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const = 0;

  /** Schedule an AND gate for computation. Since computing AND gates incurs 2
   * roundtrips, we want to batch them together to reduce the total number of
   * roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleAND(bool left, bool right) = 0;

  /**
   * Execute all the scheduled AND/batch AND computation within TWO roundtrips,
   * no matter how many AND gates are scheduled.
   */
  virtual void executeScheduledAND() = 0;

  /**
   * Get the execution result of the executed AND gate
   * @param index the index of the AND gate in the schedule
   * @return the result value
   */
  virtual bool getANDExecutionResult(uint32_t index) const = 0;

  /** Schedule a batch AND gates for computation. Since computing AND gates
   * incurs 2 roundtrips, we want to batch them together to reduce the total
   * number of roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) = 0;

  /**
   * Get the execution result of the executed AND gate
   * @param index the index of the AND gate in the schedule
   * @return the result value
   */
  virtual const std::vector<bool>& getBatchANDExecutionResult(
      uint32_t index) const = 0;

  /**
   * Compute a batch of AND gate: all inputs are private values. This batch of
   * gates will be immediately executed, incuring a roundtrip.
   * @param left the values on left input wires
   * @param right the values on right input wires
   * @return the values of the results
   */
  virtual std::vector<bool> computeBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) = 0;

  /**
   * reveal a vector of shared secret to a designated party
   * @param Id the identity of the plaintext receiver
   * @param output the plaintext output, false if this party is not the receiver
   */
  virtual std::vector<bool> revealToParty(
      int id,
      const std::vector<bool>& output) const = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine
