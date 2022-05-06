/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/types.h>
#include <type_traits>
#include "fbpcf/frontend/mpcGame.h"

namespace fbpcf::billionaire_problem {

/**
 * A variant of the classic "Millionaire Problem":
 * https://en.wikipedia.org/wiki/Yao%27s_Millionaires%27_problem
 *
 * Alice and Bob wish to determine who has the greater net worth
 * (cash + stocks + property) without revealing their exact net worths to each
 * other.
 *
 * Both parties will call `billionaireProblem()` simultaneously, passing in the
 * true value for their own assets and a dummy value for the other party's
 * assets. In this implementation, Alice receives the result of the comparison
 * (whether Bob has a higher net worth), while Bob receives a dummy boolean
 * value. The computation is carried out securely, so neither party learns the
 * value of the other's assets.
 *
 * See BillionaireProblemGameTest.cpp for a concrete usage of this class.
 */
template <int schedulerId, bool usingBatch>
class BillionaireProblemGame : public frontend::MpcGame<schedulerId> {
  using SecUnsignedInt = typename frontend::MpcGame<
      schedulerId>::template SecUnsignedInt<32, usingBatch>;

 public:
  explicit BillionaireProblemGame(
      std::unique_ptr<scheduler::IScheduler> scheduler)
      : frontend::MpcGame<schedulerId>(std::move(scheduler)) {}

  using AssetsType = typename std::
      conditional<usingBatch, std::vector<uint32_t>, uint32_t>::type;
  struct AssetsLists {
    AssetsType cash;
    AssetsType stock;
    AssetsType property;
  };

  using CompareResult =
      typename std::conditional<usingBatch, std::vector<bool>, bool>::type;

  CompareResult billionaireProblem(
      const AssetsLists& aliceAssets,
      const AssetsLists& bobAssets);

 private:
  class SecAssetsLists {
   public:
    SecAssetsLists(const AssetsLists& assets, int partyId);

    SecUnsignedInt getTotalValue();

   private:
    SecUnsignedInt cash_;
    SecUnsignedInt stock_;
    SecUnsignedInt property_;
  };
};

} // namespace fbpcf::billionaire_problem

#include "./BillionaireProblemGame_impl.h"
