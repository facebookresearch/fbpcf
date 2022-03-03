/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/io/LocalFileManager.h"
#include "folly/Format.h"
#include "folly/Random.h"

#include "fbpcf/aws/AwsSdk.h"
#include "fbpcf/aws/MockS3Client.h"
#include "fbpcf/exception/AwsException.h"
#include "fbpcf/io/S3FileManager.h"

using ::testing::_;

namespace fbpcf {
class S3FileManagerTest : public ::testing::Test {
 protected:
  void TearDown() override {
    std::remove(filePath_.c_str());
    s3Client = nullptr;
  }

  void SetUp() override {
    AwsSdk::aquire();
    s3Client = std::make_unique<MockS3Client>();
  }

  std::unique_ptr<MockS3Client> s3Client;

  const std::string testData_ = "this is test data";
  const std::string filePath_ =
      folly::sformat("./testfile_{}", folly::Random::rand32());
  const std::string kS3URL = "https://bucket.s3.region.amazonaws.com/key";
};

TEST_F(S3FileManagerTest, testReadWithException) {
  EXPECT_CALL(*s3Client, GetObject(_)).Times(1);
  S3FileManager fileManager{std::move(s3Client)};
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.read(kS3URL), AwsException);
}

TEST_F(S3FileManagerTest, testWriteWithException) {
  EXPECT_CALL(*s3Client, PutObject(_)).Times(1);
  S3FileManager fileManager{std::move(s3Client)};
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.write(kS3URL, testData_), AwsException);
}

TEST_F(S3FileManagerTest, testCopyWritesToS3) {
  EXPECT_CALL(*s3Client, PutObject(_)).Times(1);
  S3FileManager fileManager{std::move(s3Client)};
  auto localFileManager = LocalFileManager{};
  localFileManager.write(filePath_, testData_);
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.copy(filePath_, kS3URL), AwsException);
}
} // namespace fbpcf
