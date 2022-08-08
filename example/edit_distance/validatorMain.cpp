/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gflags/gflags.h>
#include "folly/init/Init.h"
#include "folly/logging/xlog.h"

#include <fbpcf/aws/AwsSdk.h>
#include "./Validator.h" // @manual

DEFINE_string(
    shares_filepath_0,
    "",
    "Input filepath for player 0 secret shares");
DEFINE_string(
    shares_filepath_1,
    "",
    "Input filepath for player 1 secret shares");
DEFINE_string(results_path, "", "File path of the correct results");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  fbpcf::AwsSdk::aquire();

  XLOGF(INFO, "Input shares path 0: {}", FLAGS_shares_filepath_0);
  XLOGF(INFO, "Input shares path 1: {}", FLAGS_shares_filepath_1);
  XLOGF(INFO, "Results path: {}", FLAGS_results_path);

  std::vector<std::string> sharesPaths = {
      FLAGS_shares_filepath_0, FLAGS_shares_filepath_1};

  XLOG(INFO, "Starting validation");

  fbpcf::edit_distance::Validator validator(sharesPaths, FLAGS_results_path);

  int code = validator.validate();
  if (code == fbpcf::edit_distance::Validator::SUCCESS) {
    XLOG(INFO, "Validation completed successfully");
  } else {
    XLOG(WARNING, "Validation failed.");
  }
  return code;
}
