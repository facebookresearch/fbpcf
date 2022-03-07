/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

inline std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransferFactory>
createClassicRcotFactory() {
  return std::make_unique<tuple_generator::oblivious_transfer::
                              EmpShRandomCorrelatedObliviousTransferFactory>(
      std::make_unique<util::AesPrgFactory>());
}

inline std::unique_ptr<IRandomCorrelatedObliviousTransferFactory>
createFerretRcotFactory(
    int64_t extendedSize = ferret::kExtendedSize,
    int64_t baseSize = ferret::kBaseSize,
    int64_t weight = ferret::kWeight) {
  return std::make_unique<
      ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
      createClassicRcotFactory(),
      std::make_unique<ferret::RcotExtenderFactory>(
          std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
          std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
              std::make_unique<ferret::SinglePointCotFactory>())),
      extendedSize,
      baseSize,
      weight);
}

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
