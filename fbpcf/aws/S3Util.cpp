/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "S3Util.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <aws/core/auth/AWSCredentials.h> // @manual
#include <aws/core/client/ClientConfiguration.h> // @manual
#include <aws/core/http/Scheme.h> // @manual
#include <aws/s3/S3Client.h> // @manual

#include <boost/algorithm/string.hpp>
#include <folly/Format.h>
#include <folly/String.h>
#include <folly/Uri.h>
#include <re2/re2.h>

#include "fbpcf/exception/AwsException.h"
#include "folly/Range.h"

namespace fbpcf::aws {
// Format:
// 1. https://bucket-name.s3.Region.amazonaws.com/key-name
// 2. https://bucket-name.s3-Region.amazonaws.com/key-name
// 3. s3://bucket-name/key-name
S3ObjectReference uriToObjectReference(std::string url) {
  std::string bucket;
  std::string region;
  auto uri = folly::Uri(url);
  auto scheme = uri.scheme();
  auto host = uri.host();
  auto path = uri.path();

  if (boost::iequals(scheme, "s3")) {
    if (!std::getenv("AWS_DEFAULT_REGION")) {
      throw AwsException{"AWS_DEFAULT_REGION not specified"};
    }
    region = std::getenv("AWS_DEFAULT_REGION");
    bucket = host;
  } else {
    // A stricter version of:
    // https://github.com/aws/aws-sdk-java/blob/c2c377058380cca07c0be9c8c6e0d7bf0b3777b8/aws-java-sdk-s3/src/main/java/com/amazonaws/services/s3/AmazonS3URI.java#L29
    //
    // Matches "bucket.s3.region.amazonaws.com" or
    // "bucket.s3-region.amazonaws.com"
    static const re2::RE2 endpoint_pattern(
        "^(?i)(.+)\\.s3[.-]([a-z0-9-]+)\\.amazonaws.com");

    // Sub-match 1: (bucket).s3.region.amazonaws.com
    // Sub-match 2: bucket.s3.(region).amazonaws.com
    if (!re2::RE2::FullMatch(host, endpoint_pattern, &bucket, &region)) {
      throw AwsException{folly::sformat(
          "Incorrect S3 URI format: {}"
          "Supported formats:"
          "1. https://bucket.s3.region.amazonaws.com/key"
          "2. https://bucket.s3-region.amazonaws.com/key"
          "3. s3://bucket/key",
          url)};
    }
  }

  if (path.length() <= 1) {
    throw AwsException{folly::sformat(
        "Incorrect S3 URI format: {}"
        "key not specified",
        url)};
  }

  // path.substr(1) to remove the first character '/'
  return S3ObjectReference{region, bucket, path.substr(1)};
}

std::unique_ptr<Aws::S3::S3Client> createS3Client(
    const S3ClientOption& option) {
  Aws::Client::ClientConfiguration config;

  if (option.region.has_value()) {
    config.region = option.region.value();
  } else if (std::getenv("AWS_DEFAULT_REGION")) {
    config.region = std::getenv("AWS_DEFAULT_REGION");
  }

  if (option.proxyHost.has_value()) {
    config.proxyHost = option.proxyHost.value();
  } else if (std::getenv("AWS_PROXY_HOST")) {
    config.proxyHost = std::getenv("AWS_PROXY_HOST");
  }

  if (option.proxyPort.has_value()) {
    config.proxyPort = option.proxyPort.value();
  } else if (std::getenv("AWS_PROXY_PORT")) {
    config.proxyPort = std::stoi(std::getenv("AWS_PROXY_PORT"));
  }

  if (option.proxyScheme.has_value()) {
    config.proxyScheme = option.proxyScheme.value() == "HTTPS"
        ? Aws::Http::Scheme::HTTPS
        : Aws::Http::Scheme::HTTP;
  } else if (std::getenv("AWS_PROXY_SCHEME")) {
    config.proxyScheme = std::string(std::getenv("AWS_PROXY_SCHEME")) == "HTTPS"
        ? Aws::Http::Scheme::HTTPS
        : Aws::Http::Scheme::HTTP;
  }

  if (option.proxySSLCertPath.has_value()) {
    config.proxySSLCertPath = option.proxySSLCertPath.value();
  } else if (std::getenv("AWS_PROXY_CERT_PATH")) {
    config.proxySSLCertPath = std::getenv("AWS_PROXY_CERT_PATH");
  }

  if (option.accessKeyId.has_value() && option.secretKey.has_value()) {
    Aws::Auth::AWSCredentials credentials(
        option.accessKeyId.value(), option.secretKey.value());
    return std::make_unique<Aws::S3::S3Client>(credentials, config);
  } else {
    return std::make_unique<Aws::S3::S3Client>(config);
  }
}
} // namespace fbpcf::aws
