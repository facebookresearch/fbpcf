/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/GCSFileManager.h"

#include <google/cloud/storage/download_options.h>
#include <memory>
#include <sstream>

#include "fbpcf/exception/GcpException.h"
#include "fbpcf/gcp/GCSUtil.h"
#include "fbpcf/io/GCSInputStream.h"

namespace gcs = ::google::cloud::storage;

namespace fbpcf {

template <class ClientCls>
std::unique_ptr<IInputStream> GCSFileManager<ClientCls>::getInputStream(
    const std::string& fileName) {
  const auto& ref = fbpcf::gcp::uriToObjectReference(fileName);

  auto outcome = GCSClient_->ReadObject(ref.bucket, ref.key, gcs::ReadRange());
  if (!outcome.status().ok()) {
    throw GcpException{outcome.status().message()};
  }

  return std::make_unique<GCSInputStream>(std::move(outcome));
}

template <class ClientCls>
std::string GCSFileManager<ClientCls>::readBytes(
    const std::string& fileName,
    std::size_t start,
    std::size_t end) {
  const auto& ref = fbpcf::gcp::uriToObjectReference(fileName);

  auto outcome =
      GCSClient_->ReadObject(ref.bucket, ref.key, gcs::ReadRange(start, end));
  if (!outcome.status().ok()) {
    throw GcpException{outcome.status().message()};
  }
  std::stringstream ss;
  ss << outcome.rdbuf();
  return ss.str();
}

template <class ClientCls>
std::string GCSFileManager<ClientCls>::read(const std::string& fileName) {
  auto stream = getInputStream(fileName);
  std::stringstream ss;
  ss << stream->get().rdbuf();
  return ss.str();
}

template <class ClientCls>
void GCSFileManager<ClientCls>::write(
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
