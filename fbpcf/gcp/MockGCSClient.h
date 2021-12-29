/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
  MOCK_CONST_METHOD3(
      ReadObject,
      gcs::ObjectReadStream(
          const std::string& bucket,
          const std::string& key,
          const gcs::ReadRange& range));

  MOCK_CONST_METHOD2(
      WriteObject,
      gcs::ObjectWriteStream(
          const std::string& bucket,
          const std::string& key));
};
} // namespace fbpcf
