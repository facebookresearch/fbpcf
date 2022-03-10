/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/rand.h>
#include <memory>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/ISinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCot.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class SinglePointCotFactory final : public ISinglePointCotFactory {
 public:
  std::unique_ptr<ISinglePointCot> create(
      std::unique_ptr<communication::IPartyCommunicationAgent>& agent)
      override {
    return std::make_unique<SinglePointCot>(agent);
  }
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
