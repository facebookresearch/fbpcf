/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../aws/AwsSdk.h"
#include "../aws/MockS3Client.h"
#include "../exception/AwsException.h"
#include "MockFileManager.h"
#include "S3FileManager.h"

using ::testing::_;

const std::string kS3URL = "https://bucket.s3.region.amazonaws.com/key";

namespace pcf {
TEST(S3FileManager, testReadWithException) {
  AwsSdk::aquire();
  auto s3Client = std::make_unique<MockS3Client>();
  EXPECT_CALL(*s3Client, GetObject(_)).Times(1);
  S3FileManager fileManager{std::move(s3Client)};
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.read(kS3URL), AwsException);
}

TEST(S3FileManager, testWriteWithException) {
  AwsSdk::aquire();
  auto s3Client = std::make_unique<MockS3Client>();
  EXPECT_CALL(*s3Client, PutObject(_)).Times(1);
  S3FileManager fileManager{std::move(s3Client)};
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.write(kS3URL, "abc"), AwsException);
}
} // namespace pcf
