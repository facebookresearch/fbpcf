/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/aws/S3Util.h"

#include <string>

#include <gtest/gtest.h>

#include "fbpcf/exception/AwsException.h"

namespace fbpcf {
TEST(S3Util, uriToObjectReference) {
  auto uri = "https://bucket.s3.region.amazonaws.com/key";
  auto ref = fbpcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(S3Util, uriToObjectReference_mixedcase) {
  auto uri = "https://Bucket.S3.region.AmazonAWS.COM/key";
  auto ref = fbpcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("Bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(S3Util, uriToObjectReference_dash) {
  auto uri = "https://bucket.s3-region.amazonaws.com/key";
  auto ref = fbpcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(S3Util, uriToObjectReference_Subfolder) {
  auto uri = "https://bucket.s3.region.amazonaws.com/folder/key";
  auto ref = fbpcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("folder/key", ref.key);
}

TEST(S3Util, uriToObjectReference_s3scheme) {
  auto uri = "s3://bucket/key";
  setenv("AWS_DEFAULT_REGION", "region", 1);
  auto ref = fbpcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(S3Util, uriToObjectReferenceException_missing_path) {
  auto uri = "https://bucket.s3.region.amazonaws.com/";
  EXPECT_THROW(fbpcf::aws::uriToObjectReference(uri), AwsException);
}

TEST(S3Util, uriToObjectReferenceException_missing_region) {
  auto uri = "https://bucket.s3.amazonaws.com/key";
  EXPECT_THROW(fbpcf::aws::uriToObjectReference(uri), AwsException);
}

TEST(S3Util, uriToObjectReferenceException_missing_bucket) {
  auto uri = "https://s3.region.amazonaws.com/key";
  EXPECT_THROW(fbpcf::aws::uriToObjectReference(uri), AwsException);
}

TEST(S3Util, uriToObjectReferenceException_s3scheme_noregion) {
  auto uri = "s3://bucket/key";
  EXPECT_THROW(fbpcf::aws::uriToObjectReference(uri), AwsException);
}

TEST(S3Util, uriToObjectReferenceException_s3scheme_nokey) {
  auto uri = "s3://bucket/";
  EXPECT_THROW(fbpcf::aws::uriToObjectReference(uri), AwsException);
}
} // namespace fbpcf
