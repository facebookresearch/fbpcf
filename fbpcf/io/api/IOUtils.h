/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace fbpcf::io {

// This chunk size has to be large enough that we don't make
// unnecessary trips to cloud storage but small enough that
// we don't cause OOM issues. This chunk size was chosen based
// on the size of our containers as well as the expected size
// of our files to fit the aforementioned constraints.
constexpr size_t kCloudBufferedReaderChunkSize = 1'073'741'824; // 2^30

// The chunk size for writing to cloud storage (S3 and GCS) must be greater than
// 5 MB per the AWS and GCS documentation. Otherwise multipart upload will fail.
// AWS Doc: https://docs.aws.amazon.com/AmazonS3/latest/userguide/qfacts.html
// GCS Doc: https://cloud.google.com/storage/quotas#requests
// The number below is 5 MB in bytes.
constexpr size_t kCloudBufferedWriterChunkSize = 5'242'880;

constexpr size_t kLocalBufferedReaderChunkSize = 4096;
constexpr size_t kLocalBufferedWriterChunkSize = 4096;

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

  static size_t getDefaultWriterChunkSizeForFile(std::string filename) {
    if (isCloudFile(filename)) {
      return kCloudBufferedWriterChunkSize;
    } else {
      return kLocalBufferedWriterChunkSize;
    }
  }

  static size_t getDefaultReaderChunkSizeForFile(std::string filename) {
    if (isCloudFile(filename)) {
      return kCloudBufferedReaderChunkSize;
    } else {
      return kLocalBufferedReaderChunkSize;
    }
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
