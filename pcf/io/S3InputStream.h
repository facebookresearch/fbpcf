#pragma once

#include <istream>

#include <aws/s3/model/GetObjectResult.h>

#include "IInputStream.h"

namespace pcf {
class S3InputStream : public IInputStream {
 public:
  explicit S3InputStream(Aws::S3::Model::GetObjectResult r) : r_{std::move(r)} {}

  std::istream& get() override;

 private:
  Aws::S3::Model::GetObjectResult r_;
};
} // namespace pcf
