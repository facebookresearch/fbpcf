/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>

#include "fbpcf/exception/GcpException.h"
#include "fbpcf/gcp/GCSUtil.h"
#include "fbpcf/io/cloud_util/GCSFileUploader.h"

namespace fbpcf::cloudio {
static const std::string FILE_TYPE = "text/csv";

void GCSFileUploader::init() {}

int32_t GCSFileUploader::upload(std::vector<char>& buf) {
  XLOG(INFO) << "Start resumable upload. ";
  const auto& ref = fbpcf::gcp::uriToObjectReference(filePath_);
  std::string bucket_ = ref.bucket;
  std::string object_ = ref.key;

  namespace gcs = ::google::cloud::storage;
  using ::google::cloud::StatusOr;
  std::string str(buf.begin(), buf.end());

  StatusOr<gcs::ObjectMetadata> object_metadata = gcsClient_->InsertObject(
      bucket_, object_, str, gcs::ContentType(FILE_TYPE));

  if (!object_metadata) {
    throw GcpException{
        "Resumable upload failed: " + object_metadata.status().message()};
    return 0;
  }
  XLOG(INFO) << " Resumable upload successful ";
  XLOG(INFO) << "Bucket: " << bucket_ << ", Object Name: " << object_;
  return str.size();
}

int GCSFileUploader::complete() {
  return 0;
}
} // namespace fbpcf::cloudio
