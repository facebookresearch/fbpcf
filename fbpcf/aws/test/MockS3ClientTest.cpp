/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/aws/MockS3Client.h"
#include "fbpcf/aws/AwsSdk.h"

#include <aws/s3/S3Client.h> // @manual
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::_;

namespace fbpcf {
TEST(MockS3Client, TestMockedClient) {
  AwsSdk::aquire();
  MockS3Client client;

  Aws::S3::Model::GetObjectRequest request;
  EXPECT_CALL(client, GetObject(_)).Times(1);
  client.GetObject(request);
}
} // namespace fbpcf
