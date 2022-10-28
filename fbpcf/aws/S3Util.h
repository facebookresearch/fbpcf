/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

#include <aws/s3/S3Client.h> // @manual

namespace fbpcf::aws {
// referencee of environment variables:
// https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-envvars.html
struct S3ClientOption {
  // AWS_DEFAULT_REGION
  std::optional<std::string> region;
  // AWS_ACCESS_KEY_ID
  std::optional<std::string> accessKeyId;
  // AWS_SECRET_ACCESS_KEY
  std::optional<std::string> secretKey;
  // AWS_PROXY_HOST
  std::optional<std::string> proxyHost;
  // AWS_PROXY_PORT
  std::optional<unsigned> proxyPort;
  // AWS_PROXY_SCHEME
  std::optional<std::string> proxyScheme;
  // AWS_PROXY_CERT_PATH
  std::optional<std::string> proxySSLCertPath;
};

struct S3ObjectReference {
  std::string region;
  std::string bucket;
  std::string key;
};

S3ObjectReference uriToObjectReference(std::string url);
std::unique_ptr<Aws::S3::S3Client> createS3Client(const S3ClientOption& option);
} // namespace fbpcf::aws
