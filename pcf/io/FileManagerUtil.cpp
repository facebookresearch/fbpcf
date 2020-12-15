#include "FileManagerUtil.h"

#include "../aws/S3Util.h"
#include "LocalFileManager.h"
#include "S3FileManager.h"

namespace pcf::io {
std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) {
  auto manager = getFileManager(fileName);
  return manager->getInputStream(fileName);
}

std::string read(const std::string& fileName) {
  auto manager = getFileManager(fileName);
  return manager->read(fileName);
}

void write(const std::string& fileName, const std::string& data) {
  auto manager = getFileManager(fileName);
  return manager->write(fileName, data);
}

FileType getFileType(const std::string& fileName) {
  // S3 file format: https://bucket-name.s3.Region.amazonaws.com/key-name
  return fileName.find("https://", 0) == 0 ? FileType::S3 : FileType::Local;
}

std::unique_ptr<pcf::IFileManager> getFileManager(const std::string& fileName) {
  auto type = getFileType(fileName);
  if (type == FileType ::S3) {
    return std::make_unique<S3FileManager>(
        // options have to be set via environment variables
        pcf::aws::createS3Client(pcf::aws::S3ClientOption{}));
  } else {
    return std::make_unique<LocalFileManager>();
  }
}
} // namespace pcf::io
