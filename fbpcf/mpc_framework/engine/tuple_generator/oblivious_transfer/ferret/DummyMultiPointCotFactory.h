/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/rand.h>
#include <memory>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

class DummyMultiPointCotFactory final : public IMultiPointCotFactory {
 public:
  explicit DummyMultiPointCotFactory(
      std::unique_ptr<util::IPrgFactory> prgFactory)
      : prgFactory_(std::move(prgFactory)) {}

  std::unique_ptr<IMultiPointCot> create(
      std::unique_ptr<communication::IPartyCommunicationAgent>& agent)
      override {
    return std::make_unique<DummyMultiPointCot>(
        agent, prgFactory_->create(util::getRandomM128iFromSystemNoise()));
  }

 private:
  std::unique_ptr<util::IPrgFactory> prgFactory_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
