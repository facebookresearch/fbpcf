/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/util/IPrg.h"

namespace fbpcf::mpc_std_lib::util {

const uint32_t RANDOM_PERMUTATION_SECURITY_BITS = 128;

/**
 *This method generates a random indexes permutation in range [0, size - 1].
 *This method required a prg to generate secure random bytes.
 */

std::vector<uint32_t> secureRandomPermutation(
    uint32_t size,
    engine::util::IPrg& prg);

} // namespace fbpcf::mpc_std_lib::util
