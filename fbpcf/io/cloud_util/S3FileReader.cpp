/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/S3FileReader.h"
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <string>
#include "fbpcf/aws/S3Util.h"
#include "fbpcf/exception/AwsException.h"

namespace fbpcf::cloudio {

std::string S3FileReader::readBytes(
    const std::string& filePath,
    std::size_t start,
    std::size_t end) {
  const auto& ref = fbpcf::aws::uriToObjectReference(filePath);
  Aws::S3::Model::GetObjectRequest request;
  std::stringstream ss;
  // NOTE: The AWS API uses a closed interval [a, b] to request a range while
  // C++ string (and most "normal" programming APIs) use a half-open interval
  // [a, b). Therefore, we subtract one here to make this usage consistent with
  // other APIs. If the user passes in readBytes(path, 0, 4), we would expect
  // to generate the range "bytes=0-3"
  ss << "bytes=" << start << '-' << (end - 1);
  request.SetBucket(ref.bucket);
  request.SetKey(ref.key);
  request.SetRange(ss.str());

  auto outcome = s3Client_->GetObject(request);
  if (!outcome.IsSuccess()) {
    throw AwsException{outcome.GetError().GetMessage()};
  }

  auto stream = outcome.GetResultWithOwnership();
  std::stringstream ss2;
  ss2 << stream.GetBody().rdbuf();
  return ss2.str();
}

size_t S3FileReader::getFileContentLength(const std::string& fileName) {
  const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
  Aws::S3::Model::HeadObjectRequest request;
  request.SetBucket(ref.bucket);
  request.SetKey(ref.key);
  auto outcome = s3Client_->HeadObject(request);

  if (outcome.IsSuccess()) {
    return outcome.GetResult().GetContentLength();
  } else {
    throw AwsException{outcome.GetError().GetMessage()};
  }
}

} // namespace fbpcf::cloudio
