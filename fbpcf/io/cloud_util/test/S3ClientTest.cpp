/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <memory>

#include "fbpcf/aws/AwsSdk.h"
#include "fbpcf/aws/MockS3Client.h"
#include "fbpcf/io/cloud_util/S3Client.h"

namespace fbpcf::cloudio {

class MockS3ClientFactory : public S3ClientFactory {
 public:
  MOCK_METHOD(
      std::shared_ptr<Aws::S3::S3Client>,
      createS3Client,
      (const fbpcf::aws::S3ClientOption&),
      ());
};

TEST(S3ClientTest, testGetInstance) {
  AwsSdk::aquire();

  bool firstTime = true;
  auto mockFactory = std::make_shared<MockS3ClientFactory>();
  EXPECT_CALL(*mockFactory, createS3Client(testing::_))
      .Times(2)
      .WillRepeatedly(
          testing::Invoke([&](const fbpcf::aws::S3ClientOption& config) {
            if (firstTime) {
              EXPECT_EQ(config.region, "us-east-1");
              firstTime = false;
            } else {
              EXPECT_EQ(config.region, "us-east-2");
            }
            return std::make_shared<MockS3Client>();
          }));

  S3Client::setS3ClientFactory(mockFactory);

  // Call getInstance with 2 regions
  auto s3Client1 =
      S3Client::getInstance(fbpcf::aws::S3ClientOption{.region = "us-east-1"});
  ASSERT_TRUE(s3Client1 != nullptr);
  ASSERT_TRUE(
      S3Client::getInstance(
          fbpcf::aws::S3ClientOption{.region = "us-east-2"}) != nullptr);
  auto s3Client2 =
      S3Client::getInstance(fbpcf::aws::S3ClientOption{.region = "us-east-1"});

  ASSERT_EQ(s3Client1, s3Client2);
  S3Client::setS3ClientFactory(nullptr);
}

} // namespace fbpcf::cloudio
