/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/S3Client.h"

#include <aws/core/Aws.h> // @manual
#include <aws/s3/S3Client.h> // @manual

#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>

namespace fbpcf::cloudio {
std::shared_ptr<S3Client> S3Client::getInstance(
    const fbpcf::aws::S3ClientOption& option) {
  /* Due to previous problems, we create a Singleton instance of the S3Client,
   * but there's a catch: we need a distinct S3Client for each region, or we
   * run into other issues. For that reason, we store this map from string to
   * S3Client with the assumption that the keys are region names. Since region
   * is optional, we also allow for a default empty string region.
   */
  static folly::Synchronized<
      folly::F14FastMap<std::string, std::shared_ptr<S3Client>>>
      m;

  std::string defaultStr{};
  auto region = option.region.value_or(defaultStr);

  return m.withWLock([&](auto& clientMap) {
    if (clientMap.find(region) == clientMap.end()) {
      std::shared_ptr<S3Client> ptr{new S3Client{option}};
      clientMap[region] = ptr;
    }

    return clientMap.at(region);
  });
}
} // namespace fbpcf::cloudio
