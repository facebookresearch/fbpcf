#pragma once

#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <gmock/gmock.h>

namespace pcf {
class MockS3Client : public Aws::S3::S3Client {
 public:
  MOCK_CONST_METHOD1(
      GetObject,
      Aws::S3::Model::GetObjectOutcome(
          const Aws::S3::Model::GetObjectRequest& request));

  MOCK_CONST_METHOD1(
      PutObject,
      Aws::S3::Model::PutObjectOutcome(
          const Aws::S3::Model::PutObjectRequest& request));
};
} // namespace pcf
