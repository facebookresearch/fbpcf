/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/io/LocalFileManager.h"
#include "folly/Format.h"
#include "folly/Random.h"

#include "fbpcf/exception/GcpException.h"
#include "fbpcf/gcp/MockGCSClient.h"
#include "fbpcf/io/GCSFileManager.h"
#include "fbpcf/io/MockFileManager.h"

using ::testing::_;

namespace fbpcf {
class GCSFileManagerTest : public ::testing::Test {
 protected:
  void TearDown() override {
    std::remove(filePath_.c_str());
  }

  void SetUp() override {
    GCSClient = std::make_shared<MockGCSClient>();
  }

  std::shared_ptr<MockGCSClient> GCSClient;
  const std::string testData_ = "this is test data";
  const std::string filePath_ =
      folly::sformat("./testfile_{}", folly::Random::rand32());
  const std::string GCSUrl = "https://storage.cloud.google.com/bucket/key";
};
TEST_F(GCSFileManagerTest, testReadWithException) {
  EXPECT_CALL(*GCSClient, ReadObject(_, _, _)).Times(1);
  GCSFileManager<MockGCSClient> fileManager{std::move(GCSClient)};

  // this call is failing because the mocked response for default
  // ObjectReadStream will have status.ok() = false
  EXPECT_THROW(fileManager.read(GCSUrl), GcpException);
}

TEST_F(GCSFileManagerTest, testWriteWithException) {
  EXPECT_CALL(*GCSClient, WriteObject(_, _)).Times(1);
  GCSFileManager<MockGCSClient> fileManager{std::move(GCSClient)};

  // this call is failing because the mocked response for default
  // ObjectWriteStream will have status.ok() = false
  EXPECT_THROW(fileManager.write(GCSUrl, testData_), GcpException);
}

TEST_F(GCSFileManagerTest, testCopyWithException) {
  auto localFileManager = LocalFileManager{};
  localFileManager.write(filePath_, testData_);
  EXPECT_CALL(*GCSClient, UploadFile(_, _, _)).Times(1);
  GCSFileManager<MockGCSClient> fileManager{std::move(GCSClient)};

  // this call is failing because the mocked response for default
  // ObjectWriteStream will have status.ok() = false
  EXPECT_THROW(fileManager.copy(filePath_, GCSUrl), GcpException);
}
} // namespace fbpcf
