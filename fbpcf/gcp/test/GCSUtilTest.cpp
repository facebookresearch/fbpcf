/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/gcp/GCSUtil.h"

#include <string>

#include <gtest/gtest.h>

#include "fbpcf/exception/GcpException.h"

namespace fbpcf {
TEST(GCSUtil, uriToObjectReference) {
  auto uri = "https://storage.cloud.google.com/bucket/key";
  auto ref = fbpcf::gcp::uriToObjectReference(uri);

  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(GCSUtil, uriToObjectReference_virtualHostStyle) {
  auto uri = "https://bucket-name.storage.googleapis.com/key-name";
  auto ref = fbpcf::gcp::uriToObjectReference(uri);

  EXPECT_EQ("bucket-name", ref.bucket);
  EXPECT_EQ("key-name", ref.key);
}

TEST(GCSUtil, uriToObjectReference_pathStyle) {
  auto uri = "https://storage.googleapis.com/bucket-name/key-name";
  auto ref = fbpcf::gcp::uriToObjectReference(uri);

  EXPECT_EQ("bucket-name", ref.bucket);
  EXPECT_EQ("key-name", ref.key);
}

TEST(GCSUtil, uriToObjectReference_Subfolder) {
  auto uri = "https://storage.cloud.google.com/bucket/folder/key";
  auto ref = fbpcf::gcp::uriToObjectReference(uri);

  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("folder/key", ref.key);
}

TEST(GCSUtil, uriToObjectReferenceException_missing_path) {
  auto uri = "https://storage.cloud.google.com/bucket/";
  EXPECT_THROW(fbpcf::gcp::uriToObjectReference(uri), GcpException);
}

TEST(GCSUtil, uriToObjectReferenceException_missing_bucket_and_path) {
  auto uri = "https://storage.cloud.google.com/";
  EXPECT_THROW(fbpcf::gcp::uriToObjectReference(uri), GcpException);
}

TEST(GCSUtil, uriToObjectReference_gsScheme) {
  auto uri = "gs://bucket/key";
  auto ref = fbpcf::gcp::uriToObjectReference(uri);

  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(GCSUtil, uriToObjectReference_gsScheme_nokey) {
  auto uri = "gs://bucket";

  EXPECT_THROW(fbpcf::gcp::uriToObjectReference(uri), GcpException);
}

} // namespace fbpcf
