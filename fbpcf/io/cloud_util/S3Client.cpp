/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/S3Client.h"

#include <aws/core/Aws.h> // @manual
#include <aws/s3/S3Client.h> // @manual

namespace fbpcf::cloudio {
S3Client& S3Client::getInstance(const fbpcf::aws::S3ClientOption& option) {
  static S3Client s3Client(option);
  return s3Client;
}
} // namespace fbpcf::cloudio
