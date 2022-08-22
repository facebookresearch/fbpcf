/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/IOUtils.h"
#include <gtest/gtest.h>
#include <string>

namespace fbpcf::io {

TEST(IOUtilsTest, testIsCloudFile) {
  bool ans =
      IOUtils::isCloudFile("https://random_bucket.us-west-2.amazonaws.com");
  EXPECT_EQ(ans, true);

  ans = IOUtils::isCloudFile("/random/local/file");
  EXPECT_EQ(ans, false);
}

TEST(IOUtilsTest, testGetDefaultWriterChunkSizeForFile) {
  auto cloudResult = IOUtils::getDefaultWriterChunkSizeForFile(
      "https://random_bucket.us-west-2.amazonaws.com");
  EXPECT_EQ(cloudResult, kCloudBufferedWriterChunkSize);

  auto localResult =
      IOUtils::getDefaultWriterChunkSizeForFile("/random/local/file");
  EXPECT_EQ(localResult, kLocalBufferedWriterChunkSize);
}

TEST(IOUtilsTest, testGetDefaultReaderChunkSizeForFile) {
  auto cloudResult = IOUtils::getDefaultReaderChunkSizeForFile(
      "https://random_bucket.us-west-2.amazonaws.com");
  EXPECT_EQ(cloudResult, kCloudBufferedReaderChunkSize);

  auto localResult =
      IOUtils::getDefaultReaderChunkSizeForFile("/random/local/file");
  EXPECT_EQ(localResult, kLocalBufferedReaderChunkSize);
}

} // namespace fbpcf::io
