/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/frontend/Bit.h"
#include "fbpcf/frontend/BitString.h"
#include "fbpcf/frontend/Int.h"
#include "fbpcf/frontend/mpcGame.h"

namespace fbpcf::frontend {

template <int schedulerId, bool usingBatch = true>
class MPCTypes {
  static const size_t charLength = 8;
  static const size_t int32Length = 32;
  static const size_t int64Length = 64;

 public:
  /** Secure private types **/
  using SecBool = typename MpcGame<schedulerId>::template SecBit<usingBatch>;

  // signed ints
  using SecChar = typename MpcGame<
      schedulerId>::template SecSignedInt<charLength, usingBatch>;
  using Sec32Int = typename MpcGame<
      schedulerId>::template SecSignedInt<int32Length, usingBatch>;
  using Sec64Int = typename MpcGame<
      schedulerId>::template SecSignedInt<int64Length, usingBatch>;

  // unsigned ints
  using SecUnsignedChar = typename MpcGame<
      schedulerId>::template SecUnsignedInt<charLength, usingBatch>;
  using SecUnsigned32Int = typename MpcGame<
      schedulerId>::template SecUnsignedInt<int32Length, usingBatch>;
  using SecUnsigned64Int = typename MpcGame<
      schedulerId>::template SecUnsignedInt<int64Length, usingBatch>;

  /** Public shared types **/
  using PubBool = typename MpcGame<schedulerId>::template PubBit<usingBatch>;

  // signed ints
  using PubChar = typename MpcGame<
      schedulerId>::template PubSignedInt<charLength, usingBatch>;
  using Pub32Int = typename MpcGame<
      schedulerId>::template PubSignedInt<int32Length, usingBatch>;
  using Pub64Int = typename MpcGame<
      schedulerId>::template PubSignedInt<int64Length, usingBatch>;

  // unsigned ints
  using PubUnsignedChar = typename MpcGame<
      schedulerId>::template PubUnsignedInt<charLength, usingBatch>;
  using PubUnsigned32Int = typename MpcGame<
      schedulerId>::template PubUnsignedInt<int32Length, usingBatch>;
  using PubUnsigned64Int = typename MpcGame<
      schedulerId>::template PubUnsignedInt<int64Length, usingBatch>;
};

} // namespace fbpcf::frontend
