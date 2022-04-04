/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "fbpcf/engine/util/AsyncBuffer.h"

namespace fbpcf::engine::util {

TEST(AsyncBufferTest, TestGetData) {
  auto index = 0;
  auto generationCount = 0;
  auto asyncBuffer =
      AsyncBuffer<int32_t>(100, [&index, &generationCount](uint64_t size) {
        generationCount++;

        return std::async(
            [&index](int size) {
              std::vector<int32_t> res;
              for (auto i = 0; i < size; ++i) {
                res.push_back(index++);
              }
              return res;
            },
            size);
      });

  // The data is generated asynchronously.
  // If n elements are requested, the generation count will be either
  // ceil(n/bufferSize) or ceil(n/bufferSize) + 1.
  EXPECT_TRUE(0 <= generationCount && generationCount <= 1);

  auto allData = asyncBuffer.getData(100);
  ASSERT_EQ(allData.size(), 100);

  EXPECT_TRUE(1 <= generationCount && generationCount <= 2);

  auto newData = asyncBuffer.getData(50);
  ASSERT_EQ(newData.size(), 50);

  allData.insert(allData.end(), newData.begin(), newData.end());

  EXPECT_TRUE(2 <= generationCount && generationCount <= 3);

  newData = asyncBuffer.getData(320);
  ASSERT_EQ(newData.size(), 320);

  allData.insert(allData.end(), newData.begin(), newData.end());

  EXPECT_TRUE(5 <= generationCount && generationCount <= 6);

  for (auto i = 0; i < 470; i++) {
    EXPECT_EQ(allData.at(i), i);
  }
}
} // namespace fbpcf::engine::util
