/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GCSUtil.h"

#include <string>

#include <folly/Format.h>
#include <folly/Uri.h>

#include "fbpcf/exception/GcpException.h"

namespace fbpcf::gcp {
// Format:
// 1. https://storage.cloud.google.com/bucket-name/key-name
GCSObjectReference uriToObjectReference(std::string url) {
  std::string bucket;
  auto uri = folly::Uri(url);
  auto host = uri.host();
  auto path = uri.path().substr(1);
  // Remove first "/" in path
  auto pos = path.find("/");
  if (pos == std::string::npos || path.substr(pos + 1).length() == 0) {
    throw GcpException{folly::sformat(
        "Incorrect GCS URI format: {}"
        "bucket/key not specified",
        url)};
  }

  bucket = path.substr(0, pos);
  // path.substr(pos+1) to remove the first character '/'
  return GCSObjectReference{bucket, path.substr(pos + 1)};
}

} // namespace fbpcf::gcp
