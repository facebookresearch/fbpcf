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

#include <aws/s3/model/CreateMultipartUploadResult.h> // @manual
#include <aws/s3/model/UploadPartRequest.h> // @manual

#include "fbpcf/aws/AwsSdk.h"
#include "fbpcf/aws/MockS3Client.h"
#include "fbpcf/io/cloud_util/S3FileUploader.h"

namespace fbpcf::cloudio {

class S3FileUploaderTest : public ::testing::Test {
 protected:
  void TearDown() override {
    s3Client = nullptr;
  }

  void SetUp() override {
    putenv(env.data());
    AwsSdk::aquire();
    s3Client = std::make_shared<MockS3Client>();
  }

  std::string env = "AWS_DEFAULT_REGION=us-east-1";
  std::shared_ptr<MockS3Client> s3Client;
};

class MockCreateMultipartUploadOutcome
    : public Aws::S3::Model::CreateMultipartUploadOutcome {
 public:
  MOCK_METHOD(bool, IsSuccess, (), (const));
};

TEST_F(S3FileUploaderTest, testUpload) {
  const std::string contents = "fileContents";

  Aws::S3::Model::CreateMultipartUploadResult createMultipartUploadRes;
  createMultipartUploadRes.SetUploadId("1");
  Aws::S3::Model::CreateMultipartUploadOutcome createMultipartUploadOutcome(
      createMultipartUploadRes);
  EXPECT_CALL(*s3Client, CreateMultipartUpload(testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(createMultipartUploadOutcome));

  Aws::S3::Model::UploadPartResult uploadPartRes;
  uploadPartRes.SetETag("etag1");
  Aws::S3::Model::UploadPartOutcome uploadPartOutcome(uploadPartRes);
  EXPECT_CALL(*s3Client, UploadPart(testing::_))
      .Times(2)
      .WillRepeatedly(testing::Invoke(
          [&](const Aws::S3::Model::UploadPartRequest& req)
              -> Aws::S3::Model::UploadPartOutcome {
            std::stringstream ss;
            ss << req.GetBody()->rdbuf();
            EXPECT_EQ(contents, ss.str());

            return uploadPartOutcome;
          }));

  Aws::S3::Model::CompleteMultipartUploadResult completeMultipartUploadRes;
  Aws::S3::Model::CompleteMultipartUploadOutcome completeMultipartUploadOutcome(
      completeMultipartUploadRes);
  EXPECT_CALL(*s3Client, CompleteMultipartUpload(testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(completeMultipartUploadOutcome));

  S3FileUploader s3FileUploader(s3Client, "s3://bucket/file");

  std::vector<char> buf(contents.begin(), contents.end());
  auto size = s3FileUploader.upload(buf);
  EXPECT_EQ(contents.length(), size);

  std::vector<char> buf2(contents.begin(), contents.end());
  size = s3FileUploader.upload(buf2);
  EXPECT_EQ(contents.length(), size);

  EXPECT_EQ(0, s3FileUploader.complete());
}

} // namespace fbpcf::cloudio
