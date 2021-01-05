/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include <memory>

#include "../EmpGame.h"

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

    return (a > b).reveal<bool>();
  }
};
} // namespace pcf
