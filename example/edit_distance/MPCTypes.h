/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "./AsciiString.h" // @manual
#include "fbpcf/frontend/mpcGame.h"

namespace fbpcf::edit_distance {

const int PLAYER0 = 0;
const int PLAYER1 = 1;

const size_t maxStringLength = 15;
const size_t charLength = 8;
const size_t int32Length = 32;

template <int schedulerId, bool usingBatch = true>
using SecBool =
    typename frontend::MpcGame<schedulerId>::template SecBit<usingBatch>;

template <int schedulerId, bool usingBatch = true>
using PubBool =
    typename frontend::MpcGame<schedulerId>::template PubBit<usingBatch>;

template <int schedulerId, bool usingBatch = true>
using PubString = AsciiString<maxStringLength, false, schedulerId, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using SecString = AsciiString<maxStringLength, true, schedulerId, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using PubChar = typename frontend::MpcGame<
    schedulerId>::template PubSignedInt<charLength, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using SecChar = typename frontend::MpcGame<
    schedulerId>::template SecSignedInt<charLength, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using Pub32Int = typename frontend::MpcGame<
    schedulerId>::template PubSignedInt<int32Length, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using Sec32Int = typename frontend::MpcGame<
    schedulerId>::template SecSignedInt<int32Length, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using SecUChar = typename frontend::MpcGame<
    schedulerId>::template SecUnsignedInt<charLength, usingBatch>;

template <int schedulerId, bool usingBatch = true>
using PubUChar = typename frontend::MpcGame<
    schedulerId>::template PubUnsignedInt<charLength, usingBatch>;

} // namespace fbpcf::edit_distance
