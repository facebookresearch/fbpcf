/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "S3FileManager.h"

#include <fstream>
#include <istream>
#include <memory>
#include <sstream>

#include <aws/s3/model/GetObjectRequest.h> // @manual
#include <aws/s3/model/PutObjectRequest.h> // @manual
#include <string>

#include "S3InputStream.h"
#include "fbpcf/aws/S3Util.h"
#include "fbpcf/exception/AwsException.h"

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

std::string S3FileManager::readBytes(
    const std::string& fileName,
    std::size_t start,
    std::size_t end) {
  const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
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

  auto stream =
      std::make_unique<S3InputStream>(outcome.GetResultWithOwnership());
  std::stringstream ss2;
  ss2 << stream->get().rdbuf();
  return ss2.str();
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
  auto ss = std::make_shared<std::stringstream>(data);
  writeDataStream(fileName, ss);
}

void S3FileManager::copy(
    const std::string& sourceFile,
    const std::string& destination) {
  std::shared_ptr<std::iostream> sourceData = std::make_shared<std::fstream>(
      sourceFile.c_str(), std::ios_base::in | std::ios_base::binary);
  writeDataStream(destination, sourceData);
}

void S3FileManager::writeDataStream(
    const std::string& fileName,
    std::shared_ptr<std::basic_iostream<char, std::char_traits<char>>>
        dataStream) {
  const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(ref.bucket);
  request.SetKey(ref.key);
  request.SetBody(dataStream);
  auto outcome = s3Client_->PutObject(request);

  if (!outcome.IsSuccess()) {
    throw AwsException{outcome.GetError().GetMessage()};
  }
}
} // namespace fbpcf
