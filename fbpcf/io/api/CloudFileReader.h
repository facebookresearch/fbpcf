/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/logging/xlog.h>
#include <cstddef>
#include <string>
#include <vector>
#include "fbpcf/exception/PcfException.h"
#include "fbpcf/io/api/IReaderCloser.h"
#include "fbpcf/io/cloud_util/CloudFileUtil.h"
#include "fbpcf/io/cloud_util/IFileReader.h"

namespace fbpcf::io {

/*
This class is the API for reading a file from cloud
storage. It can be in any supported cloud provider, but
cannot be a local file.
*/
class CloudFileReader : public IReaderCloser {
 public:
  explicit CloudFileReader(const std::string& filePath) : filePath_{filePath} {
    isClosed_ = false;
    cloudFileReader_ = fbpcf::cloudio::getCloudFileReader(filePath);
    if (cloudFileReader_ == nullptr) {
      throw fbpcf::PcfException("Unsupported cloud file reader.");
    }
    fileLength_ = cloudFileReader_->getFileContentLength(filePath);
    XLOG(INFO) << "Total file length is: " << fileLength_;
  }

  int close() override;
  size_t read(std::vector<char>& buf) override;
  bool eof() override;
  ~CloudFileReader() override;

 private:
  const std::string filePath_;
  std::size_t currentPosition_ = 0;
  std::size_t fileLength_ = 0;
  std::unique_ptr<fbpcf::cloudio::IFileReader> cloudFileReader_;
  bool isClosed_;
};

} // namespace fbpcf::io
