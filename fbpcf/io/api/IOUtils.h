/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace fbpcf::io {

class IOUtils {
 public:
  static bool isCloudFile(std::string filePath) {
    return filePath.find("https://", 0) == 0;
  }
};

} // namespace fbpcf::io
