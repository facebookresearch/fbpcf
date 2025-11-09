/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <array>
#include <memory>
#include <queue>

#include <gtest/gtest.h>

#include "fbpcf/mpc/QueueIO.h"

namespace fbpcf {
TEST(QueueIOTest, ReadAndWrite) {
  auto queueA = std::make_shared<folly::Synchronized<std::queue<char>>>();
  auto queueB = std::make_shared<folly::Synchronized<std::queue<char>>>();

  QueueIO ioA{queueA, queueB};
  QueueIO ioB{queueB, queueA};

  std::array<char, 4> a{"abc"};
  ioA.send_data(a.data(), 4);

  std::array<char, 4> b;
  ioB.recv_data(b.data(), 4);

  EXPECT_EQ(a, b);
}
} // namespace fbpcf
