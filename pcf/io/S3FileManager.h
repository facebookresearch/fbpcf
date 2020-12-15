#pragma once

#include <memory>

#include <aws/s3/S3Client.h>

#include "IFileManager.h"

namespace pcf {
class S3FileManager : public IFileManager {
 public:
  explicit S3FileManager(std::unique_ptr<Aws::S3::S3Client> client)
      : s3Client_{std::move(client)} {}

   std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) override;

  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;

 private:
  std::unique_ptr<Aws::S3::S3Client> s3Client_;
};
} // namespace pcf
