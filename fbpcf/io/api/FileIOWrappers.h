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

// This chunk size has to be large enough that we don't make
// unnecessary trips to cloud storage but small enough that
// we don't cause OOM issues. This chunk size was chosen based
// on the size of our containers as well as the expected size
// of our files to fit the aforementioned constraints.
constexpr size_t kBufferedReaderChunkSize = 1'073'741'824; // 2^30

// The chunk size for writing to cloud storage (S3 and GCS) must be greater than
// 5 MB per the AWS and GCS documentation. Otherwise multipart upload will fail.
// AWS Doc: https://docs.aws.amazon.com/AmazonS3/latest/userguide/qfacts.html
// GCS Doc: https://cloud.google.com/storage/quotas#requests
// The number below is 5 MB in bytes.
constexpr size_t kBufferedWriterChunkSize = 5'242'880;

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

 private:
  // Split an input string into component pieces given a delimiter
  static const std::vector<std::string> split(
      std::string& str,
      const std::string& delim);

  // Same as split, but specifically for comma delimiters.
  static const std::vector<std::string> splitByComma(std::string& str);
};

} // namespace fbpcf::io
