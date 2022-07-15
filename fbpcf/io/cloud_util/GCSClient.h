/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <google/cloud/storage/client.h>
#include "fbpcf/gcp/GCSUtil.h"

namespace fbpcf::cloudio {

class GCSClient {
 private:
  explicit GCSClient(const fbpcf::gcp::GCSClientOption& option) {
    GCSClient_ = fbpcf::gcp::createGCSClient();
  }

 public:
  static GCSClient& getInstance(const fbpcf::gcp::GCSClientOption& option);

  std::shared_ptr<google::cloud::storage::Client> getGCSClient() {
    return GCSClient_;
  }

 private:
  std::shared_ptr<google::cloud::storage::Client> GCSClient_;
};

} // namespace fbpcf::cloudio
