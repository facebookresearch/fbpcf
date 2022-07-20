/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace fbpcf::io {

class IOUtils {
 public:
  static bool isCloudFile(std::string filePath) {
    return filePath.find("https://", 0) == 0;
  }

  static const std::vector<std::string> splitByComma(std::string& str) {
    trim(str);
    std::vector<std::string> tokens;

    if (str == "") {
      tokens.push_back("");
      return tokens;
    }

    std::stringstream ss(str);

    while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      trim(substr);
      tokens.push_back(substr);
    }
    return tokens;
  }

 private:
  static void trim(std::string& str) {
    // Trim space from left
    str.erase(
        str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
          return !std::isspace(ch);
        }));
    // Trim spaces from right
    str.erase(
        std::find_if(
            str.rbegin(),
            str.rend(),
            [](unsigned char ch) { return !std::isspace(ch); })
            .base(),
        str.end());
  }
};

} // namespace fbpcf::io
