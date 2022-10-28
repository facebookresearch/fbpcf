/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/aws/AwsSdk.h"

#include <aws/s3/S3Client.h> // @manual
#include <gtest/gtest.h>

namespace fbpcf {
TEST(AwsSdk, aquire) {
  AwsSdk::aquire();
  Aws::S3::S3Client s3Client; // Expectation: no crash
}
} // namespace fbpcf
