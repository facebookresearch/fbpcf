/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AwsSdk.h"

#include <aws/core/Aws.h>

#include <folly/Singleton.h>

namespace fbpcf {
folly::Singleton<AwsSdk, SingletonTag> awsSdkSingleton{};
std::shared_ptr<AwsSdk> AwsSdk::aquire() {
  return awsSdkSingleton.try_get();
}

AwsSdk::AwsSdk() {
  Aws::InitAPI(options_);
}

AwsSdk::~AwsSdk() {
  Aws::ShutdownAPI(options_);
}
} // namespace fbpcf
