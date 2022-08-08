/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
#include "./EditDistanceResults.h" // @manual

namespace fbpcf::edit_distance {

class Validator {
 public:
  enum {
    SUCCESS = 0,
    SIZE_MISMATCH = 1,
    RESULT_MISMATCH = 2,
  };

  explicit Validator(
      std::vector<std::string>& outputSharePaths,
      std::string expectedOutputPath)
      : outputPaths_(outputSharePaths),
        expectedOutputPath_(expectedOutputPath) {}

  int validate();

 private:
  void appendOutputLine(
      const std::vector<std::string>& header,
      const std::vector<std::string>& parts);

  std::vector<std::string> outputPaths_;
  std::string expectedOutputPath_;

  EditDistanceResults actualResults_;
  EditDistanceResults expectedResults_;
};

} // namespace fbpcf::edit_distance
