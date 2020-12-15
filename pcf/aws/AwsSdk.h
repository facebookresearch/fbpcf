#pragma once

#include <memory>

#include <aws/core/Aws.h>

#include <folly/Singleton.h>

namespace pcf {
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
} // namespace pcf
