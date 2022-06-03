/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/S3FileUploader.h"
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <folly/logging/xlog.h>
#include "fbpcf/aws/S3Util.h"
#include "fbpcf/exception/AwsException.h"

namespace fbpcf::cloudio {

static const std::string FILE_TYPE = "text/csv";
static const int MAX_RETRY_COUNT = 3;

void S3FileUploader::init() {
  XLOG(INFO) << "Start multipart upload initialization. ";
  const auto& ref = fbpcf::aws::uriToObjectReference(filePath_);
  bucket_ = ref.bucket;
  key_ = ref.key;
  Aws::S3::Model::CreateMultipartUploadRequest request;
  request.SetBucket(bucket_);
  request.SetKey(key_);
  request.SetContentType(FILE_TYPE);

  XLOG(INFO) << "Bucket: " << bucket_ << ", Key: " << key_;

  auto createMultipartUploadOutcome = s3Client_->CreateMultipartUpload(request);

  if (createMultipartUploadOutcome.IsSuccess()) {
    uploadId_ = createMultipartUploadOutcome.GetResult().GetUploadId();
    XLOG(INFO) << "Multipart upload initialization succeed. Upload id is: "
               << uploadId_;
  } else {
    XLOG(ERR) << createMultipartUploadOutcome.GetError();
    throw AwsException{
        "Multipart upload initialization failed: " +
        createMultipartUploadOutcome.GetError().GetMessage()};
  }
}
int S3FileUploader::upload(std::vector<char>& buf) {
  XLOG(INFO) << "Start uploading part:"
             << "Part number: " << partNumber_ << "\nBucket: " << bucket_
             << "\nKey: " << key_;
  Aws::S3::Model::UploadPartRequest request;
  request.SetBucket(bucket_);
  request.SetKey(key_);
  request.SetUploadId(uploadId_);
  request.SetPartNumber(partNumber_);
  request.SetContentLength(buf.size());

  Aws::String str(buf.begin(), buf.end());
  auto inputData = Aws::MakeShared<Aws::StringStream>("UploadPartStream", str);
  request.SetBody(inputData);
  XLOG(INFO) << "Upload stream size: " << str.size();

  auto uploadPartResult = s3Client_->UploadPart(request);
  int retryCount = 0;
  while (!uploadPartResult.IsSuccess() && retryCount < MAX_RETRY_COUNT) {
    XLOG(INFO) << "Upload part " << partNumber_ << " failed. Retrying...";
    uploadPartResult = s3Client_->UploadPart(request);
    retryCount++;
  }

  if (uploadPartResult.IsSuccess()) {
    XLOG(INFO) << "Upload part " << partNumber_ << " succeeed.";
    Aws::S3::Model::CompletedPart part;
    part.SetPartNumber(request.GetPartNumber());
    part.SetETag(uploadPartResult.GetResult().GetETag());
    completedParts_.push_back(part);
    partNumber_++;
    return str.size();
  } else {
    XLOG(INFO) << "Upload part " << partNumber_ << " failed. Aborting...";
    abortUpload();
    return 0;
  }
}
int S3FileUploader::complete() {
  Aws::S3::Model::CompleteMultipartUploadRequest request;
  request.SetBucket(bucket_);
  request.SetKey(key_);
  request.SetUploadId(uploadId_);
  request.SetMultipartUpload(
      Aws::S3::Model::CompletedMultipartUpload().WithParts(completedParts_));

  auto completeMultipartUploadResult =
      s3Client_->CompleteMultipartUpload(request);
  if (completeMultipartUploadResult.IsSuccess()) {
    XLOG(INFO) << "File " << filePath_ << " uploaded successfully.";
    return 0;
  } else {
    XLOG(ERR) << "File " << filePath_ << " failed to upload.";
    XLOG(ERR) << "Error: " << completeMultipartUploadResult.GetError();
    abortUpload();
    return -1;
  }
}

void S3FileUploader::abortUpload() {
  Aws::S3::Model::AbortMultipartUploadRequest abortRequest;
  abortRequest.SetBucket(bucket_);
  abortRequest.SetKey(key_);
  abortRequest.SetUploadId(uploadId_);
  auto abortMultipartUploadResult =
      s3Client_->AbortMultipartUpload(abortRequest);
  if (abortMultipartUploadResult.IsSuccess()) {
    XLOG(INFO) << "Abort upload successed. ";
  } else {
    XLOG(ERR) << "Abort upload failed. Upload ID: " + uploadId_;
  }
}

} // namespace fbpcf::cloudio
