#include "AwsSdk.h"

#include <aws/core/Aws.h>

#include <folly/Singleton.h>

namespace pcf {
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
} // namespace pcf
