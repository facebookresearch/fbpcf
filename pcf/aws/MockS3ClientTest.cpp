#include "MockS3Client.h"

#include <aws/s3/S3Client.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AwsSdk.h"

using testing::_;

namespace pcf {
TEST(MockS3Client, TestMockedClient) {
  AwsSdk::aquire();
  MockS3Client client;

  Aws::S3::Model::GetObjectRequest request;
  EXPECT_CALL(client, GetObject(_)).Times(1);
  client.GetObject(request);
}
} // namespace pcf
