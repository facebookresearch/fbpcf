/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <memory>

#include <aws/core/Aws.h>

#include <folly/Singleton.h>

namespace fbpcf {
namespace {
struct SingletonTag {};
} // namespace

struct AwsSdk {
  ~AwsSdk();
  static std::shared_ptr<AwsSdk> aquire();

 private:
  AwsSdk();

 private:
  Aws::SDKOptions options_;

  friend class folly::Singleton<AwsSdk, SingletonTag>;
};
} // namespace fbpcf
