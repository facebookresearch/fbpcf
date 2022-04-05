/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>
#include "fbpcf/io/cloud_util/IFileReader.h"

namespace fbpcf::cloudio {
enum class CloudFileType { S3, GCS, UNKNOWN };

CloudFileType getCloudFileType(const std::string& filePath);
std::unique_ptr<IFileReader> getCloudFileReader(const std::string& filePath);
} // namespace fbpcf::cloudio
