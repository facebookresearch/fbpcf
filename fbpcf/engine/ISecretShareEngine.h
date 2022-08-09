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
   * Generate a private input wire carring party id's input
   * @param id the party that own v
   * @param v the plaintext input, this value can be std::nullopt if this
   * party doesn't own v
   * @return the ciphertext form
   */
  virtual uint64_t setIntegerInput(
      int id,
      std::optional<uint64_t> v = std::nullopt) = 0;

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
   * Generate a batch of private input wires carring party id's inputs
   * @param id the party that own v
   * @param v the plaintext inputs, the optional integers in this vector can be
   * any value if this party doesn't own v
   * @return the ciphertext form
   */
  virtual std::vector<uint64_t> setBatchIntegerInput(
      int id,
      const std::vector<uint64_t>& v) = 0;

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
   * Compute an Plus gate with two private or two public values. This operation
   * requires all parties to Plus their shares/values (That's why it is
   * symmetric). This computation can be done locally, without any interaction
   * with other parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual uint64_t computeSymmetricPlus(uint64_t left, uint64_t right)
      const = 0;

  /**
   * Compute a batch of Plus gates with two private or two public values. This
   * operation requires all parties to Plus their shares/values (That's why it
   * is symmetric). This computation can be done locally, without any
   * interaction with other parties.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual std::vector<uint64_t> computeBatchSymmetricPlus(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) const = 0;

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
   * Compute an Plus gate between a private and a public value. This operation
   * requires only one party to Plus his/her share/value (That's why it is
   * asymmetric), others only needs to output left wire value. This computation
   * can be done locally, without any interaction with other parties.
   * @param privateValue the value that is private
   * @param publicValue the value that is public
   * @return the value of the result
   */
  virtual uint64_t computeAsymmetricPlus(
      uint64_t privateValue,
      uint64_t publicValue) const = 0;

  /**
   * Compute a batch of Plus gates with a private and a public values.  This
   * operation requires only one party to Plus his/her share/value (That's why
   * it is asymmetric), others only needs to output left wire value. This
   * computation can be done locally, without any interaction with other
   * parties.
   * @param left the value that is private
   * @param right the value that is public
   * @return the value of the result
   */
  virtual std::vector<uint64_t> computeBatchAsymmetricPlus(
      const std::vector<uint64_t>& privateValue,
      const std::vector<uint64_t>& publicValue) const = 0;

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
   * Compute a Neg gate on public values, This operation
   * requires all party to flip their shares/values (That's why it is
   * symmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual uint64_t computeSymmetricNeg(uint64_t input) const = 0;

  /**
   * Compute a batch of Neg gate on public values, This operation
   * requires all party to flip their shares/values (That's why it is
   * symmetric). This computation can be done locally,
   * without any interaction with other parties.
   * @param input the value on left input wire
   * @return the value of the result
   */
  virtual std::vector<uint64_t> computeBatchSymmetricNeg(
      const std::vector<uint64_t>& input) const = 0;

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

  //======== Below are free AND computation API's: ========

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

  //======== Below are free Mult computation API's: ========

  /**
   * Compute a free Mult gate: at least one of the input is a public value, thus
   * the parties only need to multiply the secret share and the public value
   * (or two public values) locally, without interaction with peers.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the value of the result
   */
  virtual uint64_t computeFreeMult(uint64_t left, uint64_t right) const = 0;

  /**
   * Compute a batch of free Mult gate: at least one of the input is a public
   * value, thus the parties only need to multiply the secret share and the
   * public value (or two public values) locally, without interaction with
   * peers.
   * @param left the values on left input wires
   * @param right the values on right input wires
   * @return the values of the results
   */
  virtual std::vector<uint64_t> computeBatchFreeMult(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) const = 0;

  //======== Below are API's to schedule non-free AND's: ========

  /** Schedule an AND gate for computation. Since computing AND gates incurs 2
   * roundtrips, we want to batch them together to reduce the total number of
   * roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleAND(bool left, bool right) = 0;

  /** Schedule a batch AND gates for computation. Since computing AND gates
   * incurs 2 roundtrips, we want to batch them together to reduce the total
   * number of roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire. Must be same length as left
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) = 0;

  /** Schedule a composite AND gate for computation. Since computing AND gates
   * incurs 2 roundtrips, we want to batch them together to reduce the total
   * number of roundtrips.
   * @param left the value on left input wire
   * @param rights the values of the right wires to AND with the left wire
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleCompositeAND(
      bool left,
      std::vector<bool> rights) = 0;

  /** Schedule a batch composite AND gate for computation. Since computing AND
   * gates incurs 2 roundtrips, we want to batch them together to reduce the
   * total number of roundtrips.
   * @param left the values on left input wire
   * @param rights the values of the right wires to AND with the left wire. The
   * inner vector length must be the same as left length.
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleBatchCompositeAND(
      const std::vector<bool>& left,
      const std::vector<std::vector<bool>>& rights) = 0;

  //======== Below are API's to schedule non-free Mult's: ========

  /** Schedule an Mult gate for computation. Since computing AND gates incurs 2
   * roundtrips, we want to batch them together to reduce the total number of
   * roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleMult(uint64_t left, uint64_t right) = 0;

  /** Schedule a batch Mult gates for computation. Since computing AND gates
   * incurs 2 roundtrips, we want to batch them together to reduce the total
   * number of roundtrips.
   * @param left the value on left input wire
   * @param right the value on right input wire. Must be same length as left
   * @return the index of the scheduled gate, i.e. how many gates has already
   * been scheduled.
   */
  virtual uint32_t scheduleBatchMult(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) = 0;

  //======== Below are API's to execute non free AND's: ========

  /**
   * Compute a batch of AND gate: all inputs are private values. This batch of
   * gates will be immediately executed, incuring a roundtrip.
   * @param left the values on left input wires
   * @param right the values on right input wires
   * @return the values of the results
   */
  virtual std::vector<bool> computeBatchANDImmediately(
      const std::vector<bool>& left,
      const std::vector<bool>& right) = 0;

  //======== Below are API's to execute non free Mult's: ========

  /**
   * Compute a batch of Mult gate: all inputs are private values. This batch of
   * gates will be immediately executed, incuring a roundtrip.
   * @param left the values on left input wires
   * @param right the values on right input wires
   * @return the values of the results
   */
  virtual std::vector<uint64_t> computeBatchMultImmediately(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) = 0;

  //======== Below are API's to execute non free AND's and Mult's: ========

  /**
   * Execute all the scheduled AND/batch AND and MULT/batch MULT computation
   * within TWO roundtrips, no matter how many gates are scheduled.
   */
  virtual void executeScheduledOperations() = 0;

  //======== Below are API's to retrieve non-free AND results: ========

  /**
   * Get the execution result of the executed AND gate
   * @param index the index of the AND gate in the schedule
   * @return the result value
   */
  virtual bool getANDExecutionResult(uint32_t index) const = 0;

  /**
   * Get the execution result of the executed batch AND gate
   * @param index the index of the batch AND gate in the schedule
   * @return the result value
   */
  virtual const std::vector<bool>& getBatchANDExecutionResult(
      uint32_t index) const = 0;

  /**
   * Get the execution result of the executed composite AND gate
   * @param index the index of the AND gate in the schedule
   * @return the result value
   */
  virtual const std::vector<bool>& getCompositeANDExecutionResult(
      uint32_t index) const = 0;

  /**
   * Get the execution result of the executed batch composite AND gate
   * @param index the index of the AND gate in the schedule
   * @return the result value
   */
  virtual const std::vector<std::vector<bool>>&
  getBatchCompositeANDExecutionResult(uint32_t index) const = 0;

  //======== Below are API's to retrieve non-free Mult results: ========

  /**
   * Get the execution result of the executed Mult gate
   * @param index the index of the Mult gate in the schedule
   * @return the result value
   */
  virtual uint64_t getMultExecutionResult(uint32_t index) const = 0;

  /**
   * Get the execution result of the executed batch Mult gate
   * @param index the index of the batch Mult gate in the schedule
   * @return the result value
   */
  virtual const std::vector<uint64_t>& getBatchMultExecutionResult(
      uint32_t index) const = 0;

  /**
   * reveal a vector of shared secret to a designated party
   * @param Id the identity of the plaintext receiver
   * @param output the plaintext output, false if this party is not the
   * receiver
   */
  virtual std::vector<bool> revealToParty(
      int id,
      const std::vector<bool>& output) const = 0;

  /**
   * reveal a vector of shared secret to a designated party
   * @param Id the identity of the plaintext receiver
   * @param output the plaintext output, false if this party is not the
   * receiver
   */
  virtual std::vector<uint64_t> revealToParty(
      int id,
      const std::vector<uint64_t>& output) const = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine
