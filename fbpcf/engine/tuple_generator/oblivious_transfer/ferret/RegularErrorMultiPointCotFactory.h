/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/ISinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"
#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class RegularErrorMultiPointCotFactory final : public IMultiPointCotFactory {
 public:
  explicit RegularErrorMultiPointCotFactory(
      std::unique_ptr<ISinglePointCotFactory> singlePointCotFactory)
      : singlePointCotFactory_(std::move(singlePointCotFactory)) {}

  std::unique_ptr<IMultiPointCot> create(
      std::unique_ptr<communication::IPartyCommunicationAgent>& agent)
      override {
    return std::make_unique<RegularErrorMultiPointCot>(
        singlePointCotFactory_->create(agent));
  }

 private:
  std::unique_ptr<ISinglePointCotFactory> singlePointCotFactory_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
