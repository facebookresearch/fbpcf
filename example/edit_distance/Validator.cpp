/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/json.h>
#include <folly/logging/xlog.h>
#include <math.h>
#include <sstream>

#include "./Validator.h" // @manual
#include "fbpcf/io/api/FileIOWrappers.h"

namespace fbpcf::edit_distance {

int Validator::validate() {
  std::vector<folly::dynamic> shares;

  for (auto& outputPath : outputPaths_) {
    auto sharesJson = io::FileIOWrappers::readFile(outputPath);
    shares.push_back(folly::parseJson(sharesJson));
  }

  actualResults_ = EditDistanceResults(shares);

  io::FileIOWrappers::readCsv(
      expectedOutputPath_,
      [&](const std::vector<std::string>& header,
          const std::vector<std::string>& parts) {
        appendOutputLine(header, parts);
      });

  if (actualResults_.editDistances.size() !=
      expectedResults_.editDistances.size()) {
    XLOGF(
        ERR,
        "Mismatched size in edit distance results. Expected {} elements but got {}",
        expectedResults_.editDistances.size(),
        actualResults_.editDistances.size());
    return Validator::SIZE_MISMATCH;
  }

  if (actualResults_.receiverMessages.size() !=
      expectedResults_.receiverMessages.size()) {
    XLOGF(
        ERR,
        "Mismatched size in receiver message results. Expected {} elements but got {}",
        expectedResults_.receiverMessages.size(),
        actualResults_.receiverMessages.size());
    return Validator::SIZE_MISMATCH;
  }

  for (int i = 0; i < actualResults_.editDistances.size(); i++) {
    if (actualResults_.editDistances[i] != expectedResults_.editDistances[i]) {
      XLOGF(
          ERR,
          "Mismatch in results for edit distance at index {}. Expected {} but got {}",
          i,
          expectedResults_.editDistances[i],
          actualResults_.editDistances[i]);
      return Validator::RESULT_MISMATCH;
    }
    if (actualResults_.receiverMessages[i] !=
        expectedResults_.receiverMessages[i]) {
      XLOGF(
          ERR,
          "Mismatch in results for receiver message at index {}. Expected {} but got {}",
          i,
          expectedResults_.receiverMessages[i],
          actualResults_.receiverMessages[i]);
      return Validator::RESULT_MISMATCH;
    }
  }

  return Validator::SUCCESS;
}

void Validator::appendOutputLine(
    const std::vector<std::string>& header,
    const std::vector<std::string>& parts) {
  for (size_t i = 0; i < header.size(); i++) {
    const std::string& column = header[i];
    const std::string& value = parts[i];
    std::istringstream iss{value};

    int64_t parsed = 0;

    if (column == "distance") {
      iss >> parsed;
      expectedResults_.editDistances.push_back(parsed);
    } else if (column == "receiver_message") {
      expectedResults_.receiverMessages.push_back(value);
    }
  }
}

} // namespace fbpcf::edit_distance
