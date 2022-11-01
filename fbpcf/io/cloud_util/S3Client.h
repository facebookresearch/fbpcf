/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <aws/core/Aws.h> // @manual
#include <aws/s3/S3Client.h> // @manual
#include "fbpcf/aws/S3Util.h"

namespace fbpcf::cloudio {

class S3Client {
 private:
  explicit S3Client(const fbpcf::aws::S3ClientOption& option) {
    awsS3Client_ = fbpcf::aws::createS3Client(option);
  }

 public:
  static S3Client& getInstance(const fbpcf::aws::S3ClientOption& option);

  std::shared_ptr<Aws::S3::S3Client> getS3Client() {
    return awsS3Client_;
  }

 private:
  std::shared_ptr<Aws::S3::S3Client> awsS3Client_;
};

} // namespace fbpcf::cloudio
