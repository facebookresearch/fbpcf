/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GCSFileManager.h"

#include <google/cloud/storage/download_options.h>
#include <memory>
#include <sstream>

#include "GCSInputStream.h"
#include "fbpcf/exception/GcpException.h"
#include "fbpcf/gcp/GCSUtil.h"

namespace gcs = ::google::cloud::storage;

namespace fbpcf {

std::unique_ptr<IInputStream> GCSFileManager::getInputStream(
    const std::string& fileName) {
  const auto& ref = fbpcf::gcp::uriToObjectReference(fileName);

  auto outcome = GCSClient_->ReadObject(ref.bucket, ref.key);
  if (!outcome) {
    throw GcpException{outcome.status().message()};
  }

  return std::make_unique<GCSInputStream>(std::move(outcome));
}

std::string GCSFileManager::readBytes(
    const std::string& fileName,
    std::size_t start,
    std::size_t end) {
  const auto& ref = fbpcf::gcp::uriToObjectReference(fileName);

  auto outcome =
      GCSClient_->ReadObject(ref.bucket, ref.key, gcs::ReadRange(start, end));
  if (!outcome) {
    throw GcpException{outcome.status().message()};
  }
  std::stringstream ss;
  ss << outcome.rdbuf();
  return ss.str();
}

std::string GCSFileManager::read(const std::string& fileName) {
  auto stream = getInputStream(fileName);
  std::stringstream ss;
  ss << stream->get().rdbuf();
  return ss.str();
}

void GCSFileManager::write(
    const std::string& fileName,
    const std::string& data) {
  const auto& ref = fbpcf::gcp::uriToObjectReference(fileName);

  auto writer = GCSClient_->WriteObject(ref.bucket, ref.key);

  writer << data;
  writer.Close();
  if (!writer.metadata()) {
    throw GcpException{writer.metadata().status().message()};
  }
}
} // namespace fbpcf
