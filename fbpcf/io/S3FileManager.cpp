/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "S3FileManager.h"

#include <memory>
#include <sstream>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include "../aws/S3Util.h"
#include "../exception/AwsException.h"
#include "S3InputStream.h"

namespace fbpcf {

std::unique_ptr<IInputStream> S3FileManager::getInputStream(
    const std::string& fileName) {
  const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(ref.bucket);
  request.SetKey(ref.key);

  auto outcome = s3Client_->GetObject(request);
  if (!outcome.IsSuccess()) {
    throw AwsException{outcome.GetError().GetMessage()};
  }

  return std::make_unique<S3InputStream>(outcome.GetResultWithOwnership());
}

std::string S3FileManager::read(const std::string& fileName) {
  auto stream = getInputStream(fileName);
  std::stringstream ss;
  ss << stream->get().rdbuf();
  return ss.str();
}

void S3FileManager::write(
    const std::string& fileName,
    const std::string& data) {
  const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(ref.bucket);
  request.SetKey(ref.key);

  auto ss = std::make_shared<std::stringstream>(data);
  request.SetBody(ss);
  auto outcome = s3Client_->PutObject(request);

  if (!outcome.IsSuccess()) {
    throw AwsException{outcome.GetError().GetMessage()};
  }
}
} // namespace fbpcf
