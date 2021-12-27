/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "fbpcf/io/FileManagerUtil.h"

namespace fbpcf::io {
TEST(FileManagerUtilTest, TestGetS3FileType) {
  auto type =
      getFileType("https://bucket-name.s3.Region.amazonaws.com/key-name");
  EXPECT_EQ(FileType::S3, type);
}

TEST(FileManagerUtilTest, TestGetLocalFileType) {
  auto type = getFileType("/root/local");
  EXPECT_EQ(FileType::Local, type);
}
} // namespace fbpcf::io
