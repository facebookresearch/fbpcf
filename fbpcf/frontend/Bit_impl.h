/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// included for clangd resolution. Should not execute during compilation
#include <cstddef>
#include "fbpcf/frontend/Bit.h"

namespace fbpcf::frontend {

template <bool isSecret, int schedulerId, bool usingBatch>
void Bit<isSecret, schedulerId, usingBatch>::increaseReferenceCount(
    const WireType& id) const {
  if (!id.isEmpty()) {
    if constexpr (usingBatch) {
      scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .increaseReferenceCountBatch(id);
    } else {
      scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .increaseReferenceCount(id);
    }
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
void Bit<isSecret, schedulerId, usingBatch>::decreaseReferenceCount(
    const WireType& id) const {
  if (!id.isEmpty()) {
    if constexpr (usingBatch) {
      scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .decreaseReferenceCountBatch(id);
    } else {
      scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .decreaseReferenceCount(id);
    }
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
void Bit<isSecret, schedulerId, usingBatch>::moveId(
    WireType& dst,
    WireType& src) const {
  decreaseReferenceCount(dst);
  dst = src;
  src = WireType();
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>::Bit(
    const Bit<isSecret, schedulerId, usingBatch>& src) {
  id_ = src.id_;
  increaseReferenceCount(src.id_);
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>::Bit(
    Bit<isSecret, schedulerId, usingBatch>&& src) noexcept {
  moveId(id_, src.id_);
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>::Bit(ExtractedBit&& extractedBit) {
  static_assert(isSecret, "only shared secrets need recover");
  if constexpr (usingBatch) {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .recoverBooleanWireBatch(extractedBit.getValue());
  } else {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .recoverBooleanWire(extractedBit.getValue());
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>&
Bit<isSecret, schedulerId, usingBatch>::operator=(
    const Bit<isSecret, schedulerId, usingBatch>& src) {
  decreaseReferenceCount(id_);
  id_ = src.id_;
  increaseReferenceCount(src.id_);
  return *this;
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>&
Bit<isSecret, schedulerId, usingBatch>::operator=(
    Bit<isSecret, schedulerId, usingBatch>&& src) {
  moveId(id_, src.id_);
  return *this;
}

template <bool isSecret, int schedulerId, bool usingBatch>
void Bit<isSecret, schedulerId, usingBatch>::privateInput(
    const BoolType& v,
    int partyId) {
  static_assert(isSecret, "public value can't come from private input");
  decreaseReferenceCount(id_);
  if constexpr (usingBatch) {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .privateBooleanInputBatch(v, partyId);
  } else {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .privateBooleanInput(v, partyId);
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
void Bit<isSecret, schedulerId, usingBatch>::publicInput(const BoolType& v) {
  static_assert(!isSecret, "private value can't come from public input");
  decreaseReferenceCount(id_);
  if constexpr (usingBatch) {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .publicBooleanInputBatch(v);
  } else {
    id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
              .publicBooleanInput(v);
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::operator!() const {
  Bit<isSecret, schedulerId, usingBatch> rst;
  if constexpr (isSecret) {
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .notPrivateBatch(id_);
    } else {
      rst.id_ =
          scheduler::SchedulerKeeper<schedulerId>::getScheduler().notPrivate(
              id_);
    }
  } else {
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .notPublicBatch(id_);
    } else {
      rst.id_ =
          scheduler::SchedulerKeeper<schedulerId>::getScheduler().notPublic(
              id_);
    }
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::operator&(
    const Bit<isSecretOther, schedulerId, usingBatch>& other) const {
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> rst;

  if constexpr (isSecret && isSecretOther) {
    // both are secret

    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPrivateBatch(id_, other.id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPrivate(id_, other.id_);
    }
  } else if constexpr (!isSecret && !isSecretOther) {
    // both are not secret
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .publicAndPublicBatch(other.id_, id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .publicAndPublic(other.id_, id_);
    }
  } else if constexpr (isSecret) {
    // this one is secret but other is not
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPublicBatch(id_, other.id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPublic(id_, other.id_);
    }
  } else {
    // this one is not secret but other is
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPublicBatch(other.id_, id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateAndPublic(other.id_, id_);
    }
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther, size_t width>
std::array<Bit<isSecret || isSecretOther, schedulerId, usingBatch>, width>
Bit<isSecret, schedulerId, usingBatch>::operator&(
    const std::array<Bit<isSecretOther, schedulerId, usingBatch>, width>&
        otherBits) const {
  std::array<Bit<isSecret || isSecretOther, schedulerId, usingBatch>, width>
      rst;
  std::vector<WireType> otherWires(width);
  for (size_t i = 0; i < otherWires.size(); i++) {
    otherWires[i] = otherBits[i].id_;
  }

  otherWires = compositeAND<isSecretOther>(otherWires);

  for (size_t i = 0; i < otherWires.size(); i++) {
    rst[i].id_ = otherWires[i];
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
std::vector<Bit<isSecret || isSecretOther, schedulerId, usingBatch>>
Bit<isSecret, schedulerId, usingBatch>::operator&(
    const std::vector<Bit<isSecretOther, schedulerId, usingBatch>>& otherBits)
    const {
  std::vector<Bit<isSecret || isSecretOther, schedulerId, usingBatch>> rst(
      otherBits.size());
  std::vector<WireType> otherWires(otherBits.size());
  for (size_t i = 0; i < otherWires.size(); i++) {
    otherWires[i] = otherBits[i].id_;
  }

  otherWires = this->compositeAND<isSecretOther>(otherWires);

  for (size_t i = 0; i < otherWires.size(); i++) {
    rst[i].id_ = otherWires[i];
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
std::vector<
    typename Bit<isSecret || isSecretOther, schedulerId, usingBatch>::WireType>
Bit<isSecret, schedulerId, usingBatch>::compositeAND(
    const std::vector<
        typename Bit<isSecretOther, schedulerId, usingBatch>::WireType>&
        otherWires) const {
  if constexpr (isSecret && isSecretOther) {
    // both are secret

    if constexpr (usingBatch) {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .privateAndPrivateCompositeBatch(id_, otherWires);
    } else {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .privateAndPrivateComposite(id_, otherWires);
    }
  } else if constexpr (!isSecret && !isSecretOther) {
    // both are not secret
    if constexpr (usingBatch) {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .publicAndPublicCompositeBatch(id_, otherWires);
    } else {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .publicAndPublicComposite(id_, otherWires);
    }
  } else {
    // one secret, one public
    if constexpr (usingBatch) {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .privateAndPublicCompositeBatch(id_, otherWires);
    } else {
      return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
          .privateAndPublicComposite(id_, otherWires);
    }
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::operator^(
    const Bit<isSecretOther, schedulerId, usingBatch>& other) const {
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> rst;
  if constexpr (isSecret && isSecretOther) {
    // both are secret
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPrivateBatch(id_, other.id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPrivate(id_, other.id_);
    }
  } else if constexpr (!isSecret && !isSecretOther) {
    // both are not secret
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .publicXorPublicBatch(other.id_, id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .publicXorPublic(other.id_, id_);
    }
  } else if constexpr (isSecret) {
    // this one is secret but other is not
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPublicBatch(id_, other.id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPublic(id_, other.id_);
    }
  } else {
    // this one is not secret but other is
    if constexpr (usingBatch) {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPublicBatch(other.id_, id_);
    } else {
      rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                    .privateXorPublic(other.id_, id_);
    }
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::operator|(
    const Bit<isSecretOther, schedulerId, usingBatch>& other) const {
  return (*this ^ other) ^ (*this & other);
}

template <bool isSecret, int schedulerId, bool usingBatch>
typename Bit<isSecret, schedulerId, usingBatch>::BoolType
Bit<isSecret, schedulerId, usingBatch>::getValue() const {
  static_assert(!isSecret, "Can't get value on secret wires.");

  if constexpr (usingBatch) {
    return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
        .getBooleanValueBatch(id_);
  } else {
    return scheduler::SchedulerKeeper<schedulerId>::getScheduler()
        .getBooleanValue(id_);
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<false, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::openToParty(int partyId) const {
  static_assert(isSecret, "No need to open a public value.");
  Bit<false, schedulerId, usingBatch> rst;

  if constexpr (usingBatch) {
    rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                  .openBooleanValueToPartyBatch(id_, partyId);
  } else {
    rst.id_ = scheduler::SchedulerKeeper<schedulerId>::getScheduler()
                  .openBooleanValueToParty(id_, partyId);
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
typename Bit<true, schedulerId, usingBatch>::ExtractedBit
Bit<isSecret, schedulerId, usingBatch>::extractBit() const {
  static_assert(isSecret, "No need to extract a public value.");
  if constexpr (usingBatch) {
    return typename Bit<true, schedulerId, usingBatch>::ExtractedBit(
        scheduler::SchedulerKeeper<schedulerId>::getScheduler()
            .extractBooleanSecretShareBatch(id_));
  } else {
    return typename Bit<true, schedulerId, usingBatch>::ExtractedBit(
        scheduler::SchedulerKeeper<schedulerId>::getScheduler()
            .extractBooleanSecretShare(id_));
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
Bit<isSecret, schedulerId, usingBatch>
Bit<isSecret, schedulerId, usingBatch>::batchingWith(
    const std::vector<Bit<isSecret, schedulerId, usingBatch>>& others) const {
  static_assert(usingBatch, "Only batch values needs to rebatch!");
  Bit<isSecret, schedulerId, usingBatch> rst;
  std::vector<WireType> ids(others.size() + 1, id_);
  for (size_t i = 0; i < others.size(); i++) {
    ids[i + 1] = others.at(i).id_;
  }
  rst.id_ =
      scheduler::SchedulerKeeper<schedulerId>::getScheduler().batchingUp(ids);

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
std::vector<Bit<isSecret, schedulerId, usingBatch>>
Bit<isSecret, schedulerId, usingBatch>::unbatching(
    std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const {
  static_assert(usingBatch, "Only batch values needs to rebatch!");
  auto ids = scheduler::SchedulerKeeper<schedulerId>::getScheduler().unbatching(
      id_, unbatchingStrategy);
  std::vector<Bit<isSecret, schedulerId, usingBatch>> rst(
      unbatchingStrategy->size());
  for (size_t i = 0; i < ids.size(); i++) {
    rst[i].id_ = ids.at(i);
  }
  return rst;
}

} // namespace fbpcf::frontend
