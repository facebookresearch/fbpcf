/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <google/cloud/storage/client.h>

#include "fbpcf/io/cloud_util/GCSClient.h"

namespace fbpcf::cloudio {
GCSClient& GCSClient::getInstance(const fbpcf::gcp::GCSClientOption& option) {
  static GCSClient GCSClient(option);
  return GCSClient;
}
} // namespace fbpcf::cloudio
