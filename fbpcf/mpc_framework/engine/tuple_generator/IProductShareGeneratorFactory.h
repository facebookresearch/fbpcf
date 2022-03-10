/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/IProductShareGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This is the API for an object that generates product shares. namely, a party
 * holds bits a1, b1 and the other party holds bits a2, b2. This object will
 * generate the shares of a1&b2 ^ a2&b1 for the two parties
 */

class IProductShareGeneratorFactory {
 public:
  virtual ~IProductShareGeneratorFactory() = default;

  /**
   * Create a product generator with a particular party.
   * Some implementation may need party id to decide parties' roles in the
   * underlying protocol.
   */
  virtual std::unique_ptr<IProductShareGenerator> create(int id) = 0;
};

} // namespace fbpcf::engine::tuple_generator
