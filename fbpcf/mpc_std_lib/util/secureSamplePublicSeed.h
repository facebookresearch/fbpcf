/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <openssl/sha.h>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::mpc_std_lib::util {

/**
 * This method generates a public random seed for two parties. The detailed
protocol is noted below:
* 1. party 1 and party 2 sample a random string s1 and s2 respectively.
* 2. party 1 computes hash(s1||r) and share the hash with party 2. r is a random
* string as well.
* 3. party 2 shares s2 with party 1.
* 4. party 1 shares s1 and r with party 2.
* 5. party 2 verifies s1 and r against the hash received before.
* 6. the two parties outputs (s1 xor s2).
* As long as s1 is truly random or s2 is truly random, s1 xor s2 will be true
random string as well
*/

__m128i secureSamplePublicSeed(
    bool amISendingFirst,
    engine::communication::IPartyCommunicationAgent& agent);

} // namespace fbpcf::mpc_std_lib::util
