/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gtest/gtest.h>
#include <fstream>
#include <memory>

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

  static void expectFileContentsMatch(
      std::string testFilePath,
      std::string expectedFilePath) {
    auto testFile = std::make_unique<std::ifstream>(testFilePath);
    auto expectedFile = std::make_unique<std::ifstream>(expectedFilePath);

    while (!expectedFile->eof()) {
      if (testFile->eof()) {
        FAIL();
      }

      auto expected = expectedFile->get();
      auto test = testFile->get();
      EXPECT_EQ(test, expected);
    }
  }
};

} // namespace fbpcf::io
