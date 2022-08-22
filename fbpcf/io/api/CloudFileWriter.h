/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "fbpcf/io/api/IWriterCloser.h"
#include "fbpcf/io/cloud_util/CloudFileUtil.h"
#include "fbpcf/io/cloud_util/IFileUploader.h"

namespace fbpcf::io {

/*
This class is the API for writing a file to cloud
storage. It can be in any supported cloud provider, but
cannot be a local file.
*/
class CloudFileWriter : public IWriterCloser {
 public:
  explicit CloudFileWriter(const std::string& filePath) : filePath_{filePath} {
    cloudFileUploader_ = fbpcf::cloudio::getCloudFileUploader(filePath_);
    isClosed_ = false;
    filepath_ = filePath;
  }

  int close() override;
  size_t write(std::vector<char>& buf) override;
  ~CloudFileWriter() override;

 private:
  const std::string filePath_;
  std::unique_ptr<fbpcf::cloudio::IFileUploader> cloudFileUploader_;
  bool isClosed_;
};

} // namespace fbpcf::io
