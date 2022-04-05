/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>
#include <vector>

namespace fbpcf::cloudio {

class IFileUploader {
 public:
  virtual ~IFileUploader() {}

  // Initilize the upload. In multipart upload, the first step is usually
  // initiate a multipart upload, and it will return an upload ID which is
  // needed in following uploads.
  virtual void init() = 0;

  // Upload the buf to cloud
  virtual int upload(std::vector<char>& buf) = 0;

  // Complete the upload. The cloud (e.g. S3) creates the object by
  // concatenating the parts in ascending order based on the part number.
  virtual int complete() = 0;
};

} // namespace fbpcf::cloudio
