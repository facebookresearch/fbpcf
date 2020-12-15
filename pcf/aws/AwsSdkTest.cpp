#include "AwsSdk.h"

#include <aws/s3/S3Client.h>
#include <gtest/gtest.h>

namespace pcf {
TEST(AwsSdk, aquire) {
  AwsSdk::aquire();
  Aws::S3::S3Client s3Client; // Expectation: no crash
}
} // namespace pcf
