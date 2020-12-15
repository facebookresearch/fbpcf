#pragma once

#include <memory>
#include <vector>

#include <emp-sh2pc/emp-sh2pc.h>

#include "IMpcGame.h"

namespace pcf {
enum class Party { Alice = emp::ALICE, Bob = emp::BOB };
enum class Visibility {
  Public = emp::PUBLIC,
  Alice = emp::ALICE,
  Bob = emp::BOB,
  Xor = emp::XOR,
};

template <class IOChannel, class InputDataType, class OutputDataType>
class EmpGame : public IMpcGame<InputDataType, OutputDataType> {
 public:
  EmpGame(
      std::unique_ptr<IOChannel> ioChannel,
      Party party)
      : party_{party},
        ioChannel_{std::move(ioChannel)} {
    emp::setup_semi_honest(ioChannel_.get(), static_cast<int>(party));
  }

 protected:
  Party party_;

 private:
  std::unique_ptr<IOChannel> ioChannel_;
};
} // namespace pcf
