/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ProductShareGenerator.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This object always generates the product share generators that run with an
 * underlying ot
 */
template <class T>
class ProductShareGeneratorFactory final
    : public IProductShareGeneratorFactory {
 public:
  ProductShareGeneratorFactory(
      std::unique_ptr<util::IPrgFactory> prgFactory,
      std::unique_ptr<oblivious_transfer::IBidirectionObliviousTransferFactory>
          otFactory)
      : prgFactory_(std::move(prgFactory)), otFactory_(std::move(otFactory)) {}

  std::unique_ptr<IProductShareGenerator> create(int id) override {
    return std::make_unique<ProductShareGenerator>(
        prgFactory_->create(util::getRandomM128iFromSystemNoise()),
        otFactory_->create(id));
  }

 private:
  std::unique_ptr<util::IPrgFactory> prgFactory_;
  std::unique_ptr<oblivious_transfer::IBidirectionObliviousTransferFactory>
      otFactory_;
};

} // namespace fbpcf::engine::tuple_generator
