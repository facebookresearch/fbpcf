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

class S3ClientFactory {
 public:
  S3ClientFactory() {}
  virtual ~S3ClientFactory() {}
  virtual std::shared_ptr<Aws::S3::S3Client> createS3Client(
      const fbpcf::aws::S3ClientOption& option) {
    return fbpcf::aws::createS3Client(option);
  }
};

class S3Client {
 private:
  explicit S3Client(const fbpcf::aws::S3ClientOption& option) {
    awsS3Client_ = s3ClientFactory_->createS3Client(option);
  }

 public:
  static std::shared_ptr<S3Client> getInstance(
      const fbpcf::aws::S3ClientOption& option);

  std::shared_ptr<Aws::S3::S3Client> getS3Client() {
    return awsS3Client_;
  }

  static void setS3ClientFactory(std::shared_ptr<S3ClientFactory> factory) {
    s3ClientFactory_ = factory;
  }

 private:
  std::shared_ptr<Aws::S3::S3Client> awsS3Client_;
  static inline std::shared_ptr<S3ClientFactory> s3ClientFactory_ =
      std::make_shared<S3ClientFactory>();
};

} // namespace fbpcf::cloudio
