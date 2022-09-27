/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/S3Client.h"

#include <string>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <folly/container/F14Map.h>
#include <folly/Synchronized.h>

namespace fbpcf::cloudio {
std::shared_ptr<S3Client> S3Client::getInstance(const fbpcf::aws::S3ClientOption& option) {
  /* Due to previous problems, we create a Singleton instance of the S3Client,
   * but there's a catch: we need a distinct S3Client for each region, or we
   * run into other issues. For that reason, we store this map from string to
   * S3Client with the assumption that the keys are region names. Since region
   * is optional, we also allow for a default empty string region.
   * ***************************** NOT THREAD SAFE ****************************
   * NOTE: Significant refactoring is required to make this thread safe
   * Downstream usage wants a mutable reference, but a folly::Synchronized
   * RWLock will return a const ref to a reader, meaning it's hard to refactor.
   * Simply trying to use folly::Synchronized around the map isn't sufficient,
   * because we'll leak a reference to an object in the map which is unsafe.
   * ***************************** NOT THREAD SAFE ****************************
   */
  static folly::Synchronized<folly::F14FastMap<std::string, std::shared_ptr<S3Client>>> m;

  std::string defaultStr{};
  auto region = option.region.value_or(defaultStr);

  m.withWLock([&](auto& clientMap) {
    if (clientMap.find(region) == clientMap.end()) {
      std::shared_ptr<S3Client> ptr{new S3Client{option}};
      clientMap.at(region) = ptr;
    }
  });

  /* You may see this and think, "Hey, the NOT THREAD SAFE warning above is 
   * outdated, it looks like we fixed it!", but you're wrong. This still does
   * not fully solve the problem. Because the downstream consumer takes a
   * mutable reference, there's no guarantee that this is thread safe. It's
   * better than nothing, but you still shouldn't fully trust this code.
   */
  return m.wlock()->at(region);
}
} // namespace fbpcf::cloudio
