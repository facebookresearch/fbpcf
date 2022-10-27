/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

#include <google/cloud/storage/client.h> // @manual=fbsource//third-party/google-cloud-cpp/google/cloud/storage:google_cloud_cpp_storage

namespace fbpcf::gcp {
struct GCSClientOption {};

struct GCSObjectReference {
  std::string bucket;
  std::string key;
};

GCSObjectReference uriToObjectReference(std::string url);
std::unique_ptr<google::cloud::storage::Client> createGCSClient();
} // namespace fbpcf::gcp
