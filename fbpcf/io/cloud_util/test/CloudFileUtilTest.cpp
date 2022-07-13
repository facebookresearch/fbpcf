/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "fbpcf/io/cloud_util/CloudFileUtil.h"

namespace fbpcf::cloudio {
TEST(FileManagerUtilTest, TestGetCloudFileType) {
  auto s3Type1 = getCloudFileType(
      "https://bucket-name.s3.us-west-2.amazonaws.com/key-name");
  EXPECT_EQ(CloudFileType::S3, s3Type1);

  auto s3Type2 = getCloudFileType(
      "https://bucket-name.s3-us-west-2.amazonaws.com/key-name");
  EXPECT_EQ(CloudFileType::S3, s3Type2);

  auto s3Type3 = getCloudFileType("s3://bucket-name/key-name");
  EXPECT_EQ(CloudFileType::S3, s3Type3);

  auto gcsType1 =
      getCloudFileType("https://storage.cloud.google.com/bucket-name/key-name");
  EXPECT_EQ(CloudFileType::GCS, gcsType1);

  auto gcsType2 =
      getCloudFileType("https://bucket-name.storage.googleapis.com/key-name");
  EXPECT_EQ(CloudFileType::GCS, gcsType2);

  auto gcsType3 =
      getCloudFileType("https://storage.googleapis.com/bucket-name/key-name");
  EXPECT_EQ(CloudFileType::GCS, gcsType3);

  auto gcsType4 = getCloudFileType("gs://bucket-name/key-name");
  EXPECT_EQ(CloudFileType::GCS, gcsType4);

  auto unkonwnType =
      getCloudFileType("https://storage.test.com/bucket-name/key-name");
  EXPECT_EQ(CloudFileType::UNKNOWN, unkonwnType);
}
} // namespace fbpcf::cloudio
