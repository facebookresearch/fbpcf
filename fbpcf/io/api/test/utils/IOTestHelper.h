/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gtest/gtest.h>
#include <string>

namespace fbpcf::io {

class IOTestHelper {
 public:
  static void expectBufferToEqualString(
      std::vector<char>& buf,
      std::string contents,
      size_t nBytes) {
    for (int i = 0; i < nBytes; i++) {
      EXPECT_EQ(buf.at(i), contents.at(i));
    }
  }

  static std::string getBaseDirFromPath(const std::string& filePath) {
    return filePath.substr(0, filePath.rfind("/") + 1);
  }
};

} // namespace fbpcf::io
