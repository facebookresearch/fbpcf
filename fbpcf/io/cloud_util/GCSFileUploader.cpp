/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/GCSFileUploader.h"

#include <folly/logging/xlog.h>

#include "fbpcf/gcp/GCSUtil.h"

namespace fbpcf::cloudio {
namespace gcs = ::google::cloud::storage;
using ::google::cloud::StatusOr;
static const std::string FILE_TYPE = "text/csv";

void GCSFileUploader::init() {
  XLOG(INFO) << "Starting resumable upload. ";
  const auto& ref = fbpcf::gcp::uriToObjectReference(filePath_);
  std::string bucket_ = ref.bucket;
  std::string object_ = ref.key;
  XLOG(INFO) << "Bucket: " << bucket_ << ", Key: " << object_;

  stream_ =
      gcsClient_->WriteObject(bucket_, object_, gcs::ContentType(FILE_TYPE));
  sessionId_ = stream_.resumable_session_id();
}

int32_t GCSFileUploader::upload(std::vector<char>& buf) {
  stream_.write(buf.data(), buf.size());
  auto status = stream_.last_status();
  if (!status.ok()) {
    XLOG(ERR) << "Upload failed. Part number: " << partNumber_
              << ". Aborting...";
    abortUpload(sessionId_);
    return 0;
  } else {
    XLOG(INFO) << "Upload succeeded. Part number: " << partNumber_;
    XLOG(INFO) << "Bytes written: " << buf.size();
    partNumber_++;
    return buf.size();
  }
}

int GCSFileUploader::complete() {
  stream_.Close();
  StatusOr<gcs::ObjectMetadata> metadata = std::move(stream_).metadata();
  if (!metadata) {
    XLOG(ERR) << "Failed to close file " << filePath_;
    XLOG(ERR) << "Status Message: " << metadata.status().message();
    abortUpload(sessionId_);
    return 1;
  } else {
    XLOG(INFO) << "File " << filePath_ << " uploaded successfully.";
    return 0;
  }
}

google::cloud::Status GCSFileUploader::abortUpload(std::string session_id) {
  google::cloud::Status status = gcsClient_->DeleteResumableUpload(session_id);
  if (status.ok()) {
    XLOG(INFO) << "Aborted upload successfully. ";
  } else {
    XLOG(ERR) << "Abort upload failed. Message: " << status.message();
  }
  return status;
}
} // namespace fbpcf::cloudio
