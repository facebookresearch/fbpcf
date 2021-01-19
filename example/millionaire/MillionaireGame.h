/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include <memory>

#include "folly/logging/xlog.h"
#include "../../pcf/mpc/EmpGame.h"

namespace pcf {
// define the classic millionaire game
template <class IOChannel>
class MillionaireGame : public EmpGame<IOChannel, int, bool> {
 public:
  MillionaireGame(std::unique_ptr<IOChannel> io, Party party)
      : EmpGame<IOChannel, int, bool>(std::move(io), party) {}

  bool play(const int& number) override {
    emp::Integer a{64, number, emp::ALICE};
    emp::Integer b{64, number, emp::BOB};

    XLOGF(INFO, "I have money:  {}", number);
    auto result = (a > b).reveal<bool>();
    if (result) {
      XLOG(INFO) << "Alice is richer!";
    } else {
      XLOG(INFO) << "Bob is richer!";
    }

    return result;
  }
};
} // namespace pcf
