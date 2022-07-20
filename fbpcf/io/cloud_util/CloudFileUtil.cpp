/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/CloudFileUtil.h"

#include <string>

#include <re2/re2.h>

#include "fbpcf/aws/S3Util.h"
#include "fbpcf/exception/PcfException.h"
#include "fbpcf/gcp/GCSUtil.h"
#include "fbpcf/io/cloud_util/GCSFileReader.h"
#include "fbpcf/io/cloud_util/S3Client.h"
#include "fbpcf/io/cloud_util/S3FileReader.h"
#include "fbpcf/io/cloud_util/S3FileUploader.h"

namespace fbpcf::cloudio {

CloudFileType getCloudFileType(const std::string& filePath) {
  // S3 file format:
  // 1. https://bucket-name.s3.region.amazonaws.com/key-name
  // 2. https://bucket-name.s3-region.amazonaws.com/key-name
  // 3. s3://bucket-name/key-name
  // GCS file format:
  // 1. https://storage.cloud.google.com/bucket-name/key-name
  // 2. https://bucket-name.storage.googleapis.com/key-name
  // 3. https://storage.googleapis.com/bucket-name/key-name
  // 4. gs://bucket-name/key-name
  static const re2::RE2 s3Regex1(
      "https://[a-z0-9.-]+.s3.[a-z0-9-]+.amazonaws.com/.+");
  static const re2::RE2 s3Regex2(
      "https://[a-z0-9.-]+.s3-[a-z0-9-]+.amazonaws.com/.+");

  bool isS3File = re2::RE2::FullMatch(filePath, s3Regex1) ||
      re2::RE2::FullMatch(filePath, s3Regex2) || filePath.find("s3://", 0) == 0;
  if (isS3File) {
    return CloudFileType::S3;
  }

  static const re2::RE2 gcsRegex1("https://storage.cloud.google.com/.*");
  static const re2::RE2 gcsRegex2(
      "https://[a-z0-9.-]+.storage.googleapis.com/.+");
  static const re2::RE2 gcsRegex3("https://storage.googleapis.com/.*");
  bool isGCSFile = re2::RE2::FullMatch(filePath, gcsRegex1) ||
      re2::RE2::FullMatch(filePath, gcsRegex2) ||
      re2::RE2::FullMatch(filePath, gcsRegex3) ||
      filePath.find("gs://", 0) == 0;
  if (isGCSFile) {
    return CloudFileType::GCS;
  }
  return CloudFileType::UNKNOWN;
}

std::unique_ptr<IFileReader> getCloudFileReader(const std::string& filePath) {
  auto fileType = getCloudFileType(filePath);
  if (fileType == CloudFileType::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(filePath);
    return std::make_unique<S3FileReader>(fbpcf::aws::createS3Client(
        fbpcf::aws::S3ClientOption{.region = ref.region}));
  } else if (fileType == CloudFileType::GCS) {
    return std::make_unique<GCSFileReader>(fbpcf::gcp::createGCSClient());
  } else {
    throw fbpcf::PcfException("Not supported yet.");
  }
}

std::unique_ptr<IFileUploader> getCloudFileUploader(
    const std::string& filePath) {
  auto fileType = getCloudFileType(filePath);
  if (fileType == CloudFileType::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(filePath);
    return std::make_unique<S3FileUploader>(
        fbpcf::cloudio::S3Client::getInstance(
            fbpcf::aws::S3ClientOption{.region = ref.region})
            .getS3Client(),
        filePath);
  } else {
    throw fbpcf::PcfException("Not supported yet.");
  }
}

std::string getCloudProviderString(const std::string& filePath) {
  std::string provider = "";
  if (getCloudFileType(filePath) == CloudFileType::S3) {
    provider = "AWS";
  }

  if (getCloudFileType(filePath) == CloudFileType::GCS) {
    provider = "GCP";
  }
  return provider;
}

std::string getCloudStorageServiceString(const std::string& filePath) {
  std::string filetype = "";
  if (getCloudFileType(filePath) == CloudFileType::S3) {
    filetype = "S3";
  }

  if (getCloudFileType(filePath) == CloudFileType::GCS) {
    filetype = "GCS";
  }
  return filetype;
}
} // namespace fbpcf::cloudio
