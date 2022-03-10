/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/scheduler/IScheduler.h"

namespace fbpcf::scheduler {

class IArithmeticScheduler : public IScheduler {
  /**
   * An arithmetic scheduler supports all of the operations supported
   by a traditional boolean scheduler, with the addition of also handling
   integer operations (i.e. addition, multiplication, negation)
   */
 public:
  virtual ~IArithmeticScheduler() = default;

  //======== Below are input processing APIs: ========

  /**
   * Creates a private integer wire from the given value.
   * Other party's input will be ignored
   * Returns the id of the created wire
   */
  virtual WireId<IScheduler::Arithmetic> privateIntegerInput(
      uint64_t v,
      int partyId) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replication of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> privateIntegerInputBatch(
      const std::vector<uint64_t>& v,
      int partyId) = 0;

  /**
   * Create a public integer input wire with value v,
   * return its wire id.
   */
  virtual WireId<IScheduler::Arithmetic> publicIntegerInput(uint64_t v) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> publicIntegerInputBatch(
      const std::vector<uint64_t>& v) = 0;

  //======== Below are output processing APIs: ========

  /**
  * create a wire from a secret share, usually used to recover a private
   wire stored in parties' shortage; even a garbled circuit-based MPC (e.g. //
   the one provided in EMP) can still use this API. This API should be used in
   pair with extractIntegerSecretShare();
  */
  virtual WireId<IScheduler::Arithmetic> recoverIntegerWire(uint64_t v) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> recoverIntegerWireBatch(
      const std::vector<uint64_t>& v) = 0;

  /**
   * creating a wire that will store the opened secret of the input wire;
   * other parties will receive a dummy value
   */
  virtual WireId<IScheduler::Arithmetic> openIntegerValueToParty(
      WireId<IScheduler::Arithmetic> src,
      int partyId) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> openIntegerValueToPartyBatch(
      WireId<IScheduler::Arithmetic> src,
      int partyId) = 0;

  /**
   * Extract the integer share for a wire such that it can be stored
   * elsewhere.
   */
  virtual uint64_t extractIntegerSecretShare(
      WireId<IScheduler::Arithmetic> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual std::vector<uint64_t> extractIntegerSecretShareBatch(
      WireId<IScheduler::Arithmetic> id) = 0;

  /**
   * get the (plaintext) value carried by a certain wire.
   */
  virtual uint64_t getIntegerValue(WireId<IScheduler::Arithmetic> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual std::vector<uint64_t> getIntegerValueBatch(
      WireId<IScheduler::Arithmetic> id) = 0;

  //======== Below are computation APIs: ========

  // ------ Plus gates ------

  /**
   * Compute PLUS between two PRIVATE wires for integer circuits;
   * creating a wire that will store the PLUS of a PRIVATE left input and a
   * PRIVATE right input.
   */
  virtual WireId<IScheduler::Arithmetic> privatePlusPrivate(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * Compute PLUS between two series of PRIVATE wires for integer circuits;
   * creating a wire for each pair of input that will store the PLUS of a
   * PRIVATE left input and a PRIVATE right input.
   */
  virtual WireId<IScheduler::Arithmetic> privatePlusPrivateBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   *Compute PLUS between a PRIVATE and a PUBLIC wire for integer circuits;
   * creating a wire that will store the PLUS of a PRIVATE left input and a
   * PUBLIC right input.
   */
  virtual WireId<IScheduler::Arithmetic> privatePlusPublic(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> privatePlusPublicBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * Compute PLUS between two PUBLIC wires for integer circuits;
   * creating a wire that will store the PLUS of a PUBLIC left input and a
   * PUBLIC right input.
   */
  virtual WireId<IScheduler::Arithmetic> publicPlusPublic(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> publicPlusPublicBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  // ------ Mult gates ------

  /**
   * Compute MULT between two PRIVATE wires for integer circuits;
   * creating a wire that will store the MULT of a PRIVATE left input and a
   * PRIVATE right input.
   */
  virtual WireId<IScheduler::Arithmetic> privateMultPrivate(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * Compute MULT between two series of PRIVATE wires for integer circuits;
   * creating a wire for each pair of input that will store the MULT of a
   * PRIVATE left input and a PRIVATE right input.
   */
  virtual WireId<IScheduler::Arithmetic> privateMultPrivateBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   *Compute MULT between a PRIVATE and a PUBLIC wire for integer circuits;
   * creating a wire that will store the MULT of a PRIVATE left input and a
   * PUBLIC right input.
   */
  virtual WireId<IScheduler::Arithmetic> privateMultPublic(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> privateMultPublicBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * Compute MULT between two PUBLIC wires for integer circuits;
   * creating a wire that will store the MULT of a PUBLIC left input and a
   * PUBLIC right input.
   */
  virtual WireId<IScheduler::Arithmetic> publicMultPublic(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> publicMultPublicBatch(
      WireId<IScheduler::Arithmetic> left,
      WireId<IScheduler::Arithmetic> right) = 0;

  // ------ Neg gates ------

  /**
   * inverse the value on a PRIVATE wire by
   * creating a PRIVATE wire that will store the opposite value;
   */
  virtual WireId<IScheduler::Arithmetic> negPrivate(
      WireId<IScheduler::Arithmetic> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> negPrivateBatch(
      WireId<IScheduler::Arithmetic> src) = 0;

  /**
   * inverse the value on a PUBLIC wire by
   * creating a PUBLIC wire that will store the opposite value;
   */
  virtual WireId<IScheduler::Arithmetic> negPublic(
      WireId<IScheduler::Arithmetic> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  virtual WireId<IScheduler::Arithmetic> negPublicBatch(
      WireId<IScheduler::Arithmetic> src) = 0;

  //======== Below are wire management APIs: ========

  /**
   * increase the reference count to a wire. Reference count indicates how
   * many alive variables in the frontend are pointing to this wire.
   */
  using IScheduler::increaseReferenceCount;
  virtual void increaseReferenceCount(WireId<Arithmetic> src) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  using IScheduler::increaseReferenceCountBatch;
  virtual void increaseReferenceCountBatch(WireId<Arithmetic> src) = 0;

  /**
   * decrease reference count of the wire. Reference count indicates how
   * many alive variables in the frontend are pointing to this wire.
   */
  using IScheduler::decreaseReferenceCount;
  virtual void decreaseReferenceCount(WireId<Arithmetic> id) = 0;

  /**
   * same, except it process a batch of inputs. This could be useful when the
   * application is a massive replications of a small function.
   */
  using IScheduler::decreaseReferenceCountBatch;
  virtual void decreaseReferenceCountBatch(WireId<Arithmetic> id) = 0;
};

} // namespace fbpcf::scheduler
