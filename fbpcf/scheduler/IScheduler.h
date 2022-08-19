/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace fbpcf::scheduler {
/**
 * A scheduler is the object that process all frontend computation.
 * It takes in computation requests from the frontend, (possibly passes these
 * requests to another backend), and return that value back to frontend.
 */

class IScheduler {
 public:
  enum WireType {
    Boolean,
    Arithmetic,
  };
  template <WireType T>
  class WireId;

  virtual ~IScheduler() = default;

  //======== Below are input processing APIs: ========

  /**
   * create a private boolean input wire with value v from the party with
   * partyID, return its wire id. Other party's input will be ignored.
   */
  virtual WireId<Boolean> privateBooleanInput(bool v, int partyId) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replication of a small function.
   */
  virtual WireId<Boolean> privateBooleanInputBatch(
      const std::vector<bool>& v,
      int partyId) = 0;

  /**
   * create a public boolean input wire with value v,
   * return its wire id.
   */
  virtual WireId<Boolean> publicBooleanInput(bool v) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> publicBooleanInputBatch(
      const std::vector<bool>& v) = 0;

  /**
  * create a wire from a secret share, usually used to recover a private
   wire stored in parties' shortage; even a garbled circuit-based MPC (e.g. //
   the one provided in EMP) can still use this API. This API should be used in
   pair with extractBooleanSecretShare();
  */
  virtual WireId<Boolean> recoverBooleanWire(bool v) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> recoverBooleanWireBatch(
      const std::vector<bool>& v) = 0;

  //======== Below are output processing APIs: ========
  /**
   * creating a wire that will store the opened secret of the input wire;
   * other parties will receive a dummy value
   */
  virtual WireId<Boolean> openBooleanValueToParty(
      WireId<Boolean> src,
      int partyId) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> openBooleanValueToPartyBatch(
      WireId<Boolean> src,
      int partyId) = 0;

  /**
   * Extract the boolean share for a wire such that it can be stored
   * elsewhere.
   */
  virtual bool extractBooleanSecretShare(WireId<Boolean> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual std::vector<bool> extractBooleanSecretShareBatch(
      WireId<Boolean> id) = 0;

  /**
   * get the (plaintext) value carried by a certain wire.
   */
  virtual bool getBooleanValue(WireId<Boolean> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual std::vector<bool> getBooleanValueBatch(WireId<Boolean> id) = 0;

  //======== Below are computation APIs: ========

  // ------ AND gates ------

  /**
   * Compute AND between two PRIVATE wires for boolean circuits;
   * creating a wire that will store the AND of a PRIVATE left input and a
   * PRIVATE right input.
   */
  virtual WireId<Boolean> privateAndPrivate(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * Compute ANDs between two series of PRIVATE wires for boolean circuits;
   * creating a wire for each pair of input that will store the AND of a PRIVATE
   * left input and a PRIVATE right input.
   */
  virtual WireId<Boolean> privateAndPrivateBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   *Compute AND between a PRIVATE and a PUBLIC wire for boolean circuits;
   * creating a wire that will store the AND of a PRIVATE left input and a
   * PUBLIC right input.
   */
  virtual WireId<Boolean> privateAndPublic(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> privateAndPublicBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * Compute AND between two PUBLIC wires for boolean circuits;
   * creating a wire that will store the AND of a PUBLIC left input and a
   * PUBLIC right input.
   */
  virtual WireId<Boolean> publicAndPublic(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> publicAndPublicBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  // ------ Composite AND gates ------

  /**
   * Compute AND between a single PRIVATE bit and vector of PRIVATE bits
   * Output will be a vector of bits corresponding to the AND of each bit in
   * rights with left bit
   */
  virtual std::vector<WireId<Boolean>> privateAndPrivateComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  /**
   * Compute AND between vector of PRIVATE bits and vector of vector of PRIVATE
   * bits; Output will be a vector of vector of bits corresponding to the AND of
   * each bit vector in rights with left corresponding bit.
   * i.e. batchCompositeAnd([a,b], [[x1, x2], [y1, y2]]) =
   *        [[a & x1, b & x2], [a & y1, b & y2]]
   */
  virtual std::vector<WireId<Boolean>> privateAndPrivateCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  /**
   * Compute AND between a single bit and vector of bits. Either the single bit
   * is public or all bits in the vector are public. Output will be a vector of
   * bits corresponding to the AND of each bit in rights with left bit
   */
  virtual std::vector<WireId<Boolean>> privateAndPublicComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  /**
   * Compute AND between vector of bits and vector of vector of bits; One side
   * is all public bits. Output will be a vector of vector of bits corresponding
   * to the AND of each bit vector in rights with left corresponding bit.
   * i.e. batchCompositeAnd([a,b], [[x1, x2], [y1, y2]]) =
   *        [[a & x1, b & x2], [a & y1, b & y2]]
   */
  virtual std::vector<WireId<Boolean>> privateAndPublicCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  /**
   * Compute AND between a single PUBLIC bit and vector of PUBLIC bits
   * Output will be a vector of bits corresponding to the AND of each bit in
   * rights with left bit
   */
  virtual std::vector<WireId<Boolean>> publicAndPublicComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  /**
   * Compute AND between vector of bits and vector of vector of bits; All bits
   * are public. Output will be a vector of vector of bits corresponding to the
   * AND of each bit vector in rights with left corresponding bit.
   * i.e. batchCompositeAnd([a,b], [[x1, x2], [y1, y2]]) =
   *        [[a & x1, b & x2], [a & y1, b & y2]]
   */
  virtual std::vector<WireId<Boolean>> publicAndPublicCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) = 0;

  // ------ XOR gates ------

  /**
   * Compute XOR between two PRIVATE wires for boolean circuits;
   * creating a wire that will store the XOR of a PRIVATE left input and a
   * PRIVATE right input.
   */
  virtual WireId<Boolean> privateXorPrivate(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> privateXorPrivateBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * Compute XOR between a PRIVATE and a PUBLIC wire for boolean circuits;
   * creating a wire that will store the XOR of a PRIVATE left input and a
   * PUBLIC right input.
   */
  virtual WireId<Boolean> privateXorPublic(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> privateXorPublicBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * Compute XOR between two PUBLIC wires for boolean circuits;
   * creating a wire that will store the XOR of a PUBLIC left input and a
   * PUBLIC right input.
   */
  virtual WireId<Boolean> publicXorPublic(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> publicXorPublicBatch(
      WireId<Boolean> left,
      WireId<Boolean> right) = 0;

  // ------ Not gates ------

  /**
   * inverse the value on a PRIVATE wire by
   * creating a PRIVATE wire that will store the opposite value;
   */
  virtual WireId<Boolean> notPrivate(WireId<Boolean> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> notPrivateBatch(WireId<Boolean> src) = 0;

  /**
   * inverse the value on a PUBLIC wire by
   * creating a PUBLIC wire that will store the opposite value;
   */
  virtual WireId<Boolean> notPublic(WireId<Boolean> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<Boolean> notPublicBatch(WireId<Boolean> src) = 0;

  //======== Below are wire management APIs: ========

  /**
   * increase the reference count to a wire. Reference count indicates how
   * many alive variables in the frontend are pointing to this wire.
   */
  virtual void increaseReferenceCount(WireId<Boolean> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual void increaseReferenceCountBatch(WireId<Boolean> src) = 0;

  /**
   * decrease reference count of the wire. Reference count indicates how
   * many alive variables in the frontend are pointing to this wire.
   */
  virtual void decreaseReferenceCount(WireId<Boolean> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual void decreaseReferenceCountBatch(WireId<Boolean> id) = 0;

  //======== Below are rebatching APIs: ========

  // band a number of batches into one batch.
  virtual WireId<Boolean> batchingUp(std::vector<WireId<Boolean>> src) = 0;

  // decompose a batch of values into several smaller batches.
  virtual std::vector<WireId<Boolean>> unbatching(
      WireId<Boolean> src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) = 0;

  //======== Below are miscellaneous APIs: ========
  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;

  /**
   * Get amount of boolean gates executed. non-free gate count is the single
   * index to look at for cost estimation. Roughly speaking, the cost grows
   * linearly with # of non-free gates.
   * Free gates are those can be executed locally.
   * Non-free gates are those need communication to execute.
   * @return a pair of (non-free gates, free gates) executed.
   */
  std::pair<uint64_t, uint64_t> getGateStatistics() const {
    return {nonFreeGates_, freeGates_};
  }

  /**
   * @return a pair of the number of wires (allocated, deallocated).
   */
  virtual std::pair<uint64_t, uint64_t> getWireStatistics() const = 0;

  /**
   * Get the batch size of a Boolean wire by inquring the wireKeeper.
   * @return the batch size of a wire.
   */
  virtual size_t getBatchSize(
      IScheduler::WireId<IScheduler::Boolean> id) const = 0;

  /**
   * Get the batch size of an Arithmetic wire by inquring the wireKeeper.
   * @return the batch size of a wire.
   */
  virtual size_t getBatchSize(
      IScheduler::WireId<IScheduler::Arithmetic> id) const = 0;

 protected:
  uint64_t nonFreeGates_ = 0;
  uint64_t freeGates_ = 0;
};

template <IScheduler::WireType T>
class IScheduler::WireId {
 public:
  WireId<T>() = default;
  explicit WireId<T>(uint64_t id) : id_(id) {}
  uint64_t getId() const {
    return *id_;
  }

  WireType getType() const {
    return T;
  }

  bool isEmpty() const {
    return !id_.has_value();
  }

 private:
  std::optional<uint64_t> id_ = std::nullopt;
};

// this object is a helper to get "global" schedulers.
// the frontend types (e.g. Bit) has a template parameter describing which
// scheduler it should use.
// This object is the holder of those schedulers.
template <int schedulerId>
class SchedulerKeeper {
 public:
  static void setScheduler(std::unique_ptr<IScheduler> scheduler) {
    scheduler_ = std::move(scheduler);
  }

  static void freeScheduler() {
    scheduler_ = nullptr;
  }

  static std::pair<uint64_t, uint64_t> getTrafficStatistics() {
    return scheduler_->getTrafficStatistics();
  }

  static std::pair<uint64_t, uint64_t> getGateStatistics() {
    return scheduler_->getGateStatistics();
  }

  static std::pair<uint64_t, uint64_t> getWireStatistics() {
    return scheduler_->getWireStatistics();
  }

 protected:
  IScheduler& getScheduler() const {
    return *scheduler_;
  }

 private:
  inline static std::unique_ptr<IScheduler> scheduler_;
};

} // namespace fbpcf::scheduler
