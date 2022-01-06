/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gmock/gmock.h>
#include <google/cloud/storage/client.h>
#include <google/cloud/storage/download_options.h>
#include <google/cloud/storage/object_read_stream.h>
#include <google/cloud/storage/object_write_stream.h>

namespace gcs = ::google::cloud::storage;

namespace fbpcf {
class MockGCSClient : public gcs::Client {
 public:
  MOCK_METHOD(
      gcs::ObjectReadStream,
      ReadObject,
      (const std::string, const std::string, const gcs::ReadRange));

  MOCK_METHOD(
      gcs::ObjectWriteStream,
      WriteObject,
      (const std::string, const std::string));
};
} // namespace fbpcf
