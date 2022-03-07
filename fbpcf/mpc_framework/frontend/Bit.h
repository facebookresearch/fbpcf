/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include "fbpcf/mpc_framework/scheduler/IScheduler.h"

namespace fbpcf::mpc_framework::frontend {

template <bool isSecret, int schedulerId, bool usingBatch = false>
class Bit : public scheduler::SchedulerKeeper<schedulerId> {
  using BoolType =
      typename std::conditional<usingBatch, std::vector<bool>, bool>::type;
  using WireType =
      scheduler::IScheduler::WireId<scheduler::IScheduler::Boolean>;

 public:
  class ExtractedBit {
   public:
    ExtractedBit() = default;
    explicit ExtractedBit(const BoolType& v) : v_(v) {}
    BoolType getValue() const {
      return v_;
    }

   private:
    BoolType v_;
  };

  /**
   * Create an uninitialized bit
   */
  Bit() : id_() {}

  /**
   * Create a bit with a public value v.
   */
  explicit Bit(const BoolType& v) {
    static_assert(!isSecret);
    publicInput(v);
  }

  /**
   * Create a bit from an extracted value.
   */
  explicit Bit(ExtractedBit&& extractedBit);

  /**
   * Create a bit with a private value v from party corresponding to
   * partyId; other parties input will be ignored.
   */
  Bit(const BoolType& v, int partyId) {
    static_assert(isSecret);
    privateInput(v, partyId);
  }

  Bit(const Bit<isSecret, schedulerId, usingBatch>& src);

  Bit(Bit<isSecret, schedulerId, usingBatch>&& src) noexcept;

  Bit<isSecret, schedulerId, usingBatch>& operator=(
      const Bit<isSecret, schedulerId, usingBatch>& src);

  Bit<isSecret, schedulerId, usingBatch>& operator=(
      Bit<isSecret, schedulerId, usingBatch>&& src);

  ~Bit<isSecret, schedulerId, usingBatch>() {
    decreaseReferenceCount(id_);
  }

  /**
   * Set this bit with a public value v
   */
  void publicInput(const BoolType& v);

  /**
   * Set this bit with a private value v from party corresponding to partyId;
   * other parties input will be ignored.
   */
  void privateInput(const BoolType& v, int partyId);

  Bit<isSecret, schedulerId, usingBatch> operator!() const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator&(
      const Bit<isSecretOther, schedulerId, usingBatch>& other) const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator^(
      const Bit<isSecretOther, schedulerId, usingBatch>& other) const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator||(
      const Bit<isSecretOther, schedulerId, usingBatch>& other) const;

  /**
   * Create a new bit that will carry the plaintext signal of this bit.
   * However only party with partyId will receive the actual value, other
   * parties will receive a dummy value.
   */
  Bit<false, schedulerId, usingBatch> openToParty(int partyId) const;

  /**
   * get the plaintext value associated with this bit
   */
  BoolType getValue() const;

  /**
   * extract this party's share of this bit
   */
  typename Bit<true, schedulerId, usingBatch>::ExtractedBit extractBit() const;

 private:
  void increaseReferenceCount(const WireType& v) const;
  void decreaseReferenceCount(const WireType& v) const;
  void moveId(WireType& dst, WireType& src) const;

  // this variable records the wire index
  WireType id_{};

  friend class Bit<!isSecret, schedulerId, usingBatch>;
};

} // namespace fbpcf::mpc_framework::frontend

#include "fbpcf/mpc_framework/frontend/Bit_impl.h"
