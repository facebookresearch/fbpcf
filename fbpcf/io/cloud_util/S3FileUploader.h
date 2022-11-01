/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <aws/s3/S3Client.h> // @manual
#include <aws/s3/model/CompletedPart.h> // @manual
#include <memory>
#include "fbpcf/io/cloud_util/IFileUploader.h"

namespace fbpcf::cloudio {

class S3FileUploader : public IFileUploader {
 public:
  explicit S3FileUploader(
      std::shared_ptr<Aws::S3::S3Client> s3Client,
      const std::string& filePath)
      : s3Client_{std::move(s3Client)}, filePath_{filePath} {
    init();
  }
  int upload(std::vector<char>& buf) override;
  int complete() override;

 private:
  void init() override;
  void abortUpload();
  std::shared_ptr<Aws::S3::S3Client> s3Client_;
  const std::string filePath_;
  std::string bucket_;
  std::string key_;
  std::string uploadId_;
  std::size_t partNumber_ = 1;
  Aws::Vector<Aws::S3::Model::CompletedPart> completedParts_;
};

} // namespace fbpcf::cloudio
