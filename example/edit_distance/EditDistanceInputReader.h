/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
#include "fbpcf/io/api/FileIOWrappers.h"

namespace fbpcf::edit_distance {

class EditDistanceInputReader {
 public:
  explicit EditDistanceInputReader(
      std::string dataFilepath,
      std::string paramsFilePath);

  const std::vector<std::string>& getWords() const {
    return words_;
  }

  const std::vector<std::string>& getGuesses() const {
    return guesses_;
  }

  const std::vector<std::string>& getSenderMessages() const {
    return senderMessages_;
  }

  int getThreshold() const {
    return threshold_;
  }

  int getDeleteCost() const {
    return deleteCost_;
  }

  int getInsertCost() const {
    return insertCost_;
  }

  int getNumRows() const {
    return numRows_;
  }

 private:
  void addFromCSV(
      const std::vector<std::string>& header,
      const std::vector<std::string>& parts);
  std::vector<std::string> words_;
  std::vector<std::string> guesses_;
  std::vector<std::string> senderMessages_;

  int threshold_;
  int deleteCost_;
  int insertCost_;
  int numRows_ = 0;
};

} // namespace fbpcf::edit_distance
