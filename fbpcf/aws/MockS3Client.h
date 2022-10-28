/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <aws/s3/S3Client.h> // @manual
#include <aws/s3/model/GetObjectRequest.h> // @manual
#include <aws/s3/model/PutObjectRequest.h> // @manual
#include <gmock/gmock.h>

namespace fbpcf {
class MockS3Client : public Aws::S3::S3Client {
 public:
  MOCK_CONST_METHOD1(
      GetObject,
      Aws::S3::Model::GetObjectOutcome(
          const Aws::S3::Model::GetObjectRequest& request));

  MOCK_CONST_METHOD1(
      PutObject,
      Aws::S3::Model::PutObjectOutcome(
          const Aws::S3::Model::PutObjectRequest& request));
};
} // namespace fbpcf
