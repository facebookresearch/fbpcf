/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "./EditDistanceInputReader.h" // @manual
#include <sstream>

namespace fbpcf::edit_distance {

EditDistanceInputReader::EditDistanceInputReader(
    std::string dataFilepath,
    std::string paramsFilePath) {
  auto readLine = [&](const std::vector<std::string>& header,
                      const std::vector<std::string>& parts) {
    ++numRows_;
    return addFromCSV(header, parts);
  };
  io::FileIOWrappers::readCsv(dataFilepath, readLine);

  auto readParams = [&](const std::vector<std::string>& header,
                        const std::vector<std::string>& parts) {
    return addFromCSV(header, parts);
  };
  io::FileIOWrappers::readCsv(paramsFilePath, readParams);

  if (words_.empty()) {
    words_ = std::vector<std::string>(numRows_, "");
  }

  if (guesses_.empty()) {
    guesses_ = std::vector<std::string>(numRows_, "");
  }

  if (senderMessages_.empty()) {
    senderMessages_ = std::vector<std::string>(numRows_, "");
  }
}

void EditDistanceInputReader::addFromCSV(
    const std::vector<std::string>& header,
    const std::vector<std::string>& parts) {
  for (size_t i = 0; i < header.size(); i++) {
    std::string column = header[i];
    std::string value = parts[i];
    std::istringstream iss{value};

    int64_t parsed = 0;
    if (column == "word") {
      words_.push_back(value);
    } else if (column == "sender_message") {
      senderMessages_.push_back(value);
    } else if (column == "guess") {
      guesses_.push_back(value);
    } else if (column == "threshold") {
      iss >> parsed;
      threshold_ = parsed;
    } else if (column == "delete_cost") {
      iss >> parsed;
      deleteCost_ = parsed;
    } else if (column == "insert_cost") {
      iss >> parsed;
      insertCost_ = parsed;
    }
  }
}
} // namespace fbpcf::edit_distance
