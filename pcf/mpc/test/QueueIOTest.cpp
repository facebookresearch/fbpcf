#include <array>
#include <memory>
#include <queue>
#include <vector>

#include <gtest/gtest.h>

#include "../QueueIO.h"

namespace pcf {
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
} // namespace pcf
