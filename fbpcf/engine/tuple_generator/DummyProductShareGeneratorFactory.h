/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/DummyProductShareGenerator.h"
#include "fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 * This object always generates Dummy product share generators
 */
class DummyProductShareGeneratorFactory final
    : public IProductShareGeneratorFactory {
 public:
  explicit DummyProductShareGeneratorFactory(
      communication::IPartyCommunicationAgentFactory& factory)
      : factory_(factory) {}

  std::unique_ptr<IProductShareGenerator> create(int id) override {
    return std::make_unique<DummyProductShareGenerator>(
        factory_.create(id, "Dummy_Product_Generator_Traffic"));
  }

 private:
  communication::IPartyCommunicationAgentFactory& factory_;
};

} // namespace fbpcf::engine::tuple_generator::insecure
