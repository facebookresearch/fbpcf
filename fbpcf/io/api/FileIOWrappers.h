/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <re2/re2.h>
#include <cstddef>
#include <string>
#include <vector>

namespace fbpcf::io {

// This class provides wrappers to the raw APIs to
// do common operations like upload an entire file or
// retrieve an entire file.
class FileIOWrappers {
 public:
  static std::string readFile(const std::string& srcPath);
  static void writeFile(
      const std::string& destPath,
      const std::string& content);
  static void transferFileInParts(
      const std::string& srcPath,
      const std::string& destPath);

  // Reads a csv from the given file, calling the given function for each line
  // Returns true on success, false on failure
  static bool readCsv(
      const std::string& srcPath,
      std::function<void(
          const std::vector<std::string>& header,
          const std::vector<std::string>& parts)> readLine,
      std::function<void(const std::vector<std::string>&)> processHeader =
          [](auto) {});
};

} // namespace fbpcf::io
