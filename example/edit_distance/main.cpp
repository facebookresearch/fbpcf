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
#include "./MPCTypes.h" // @manual
#include "./MainUtil.h" // @manual

DEFINE_int32(party, 0, "0 = player 0, 1 = player 1");
DEFINE_string(server_ip, "127.0.0.1", "Server's IP Address");
DEFINE_int32(
    port,
    10000,
    "Network port for establishing connection to other player");
DEFINE_string(
    input_path,
    "",
    "Input path of player data for edit distance game");
DEFINE_string(
    input_params,
    "",
    "Input path for parameters for edit distance game. Should be the same as other players.");
DEFINE_string(
    output_file_path,
    "",
    "Path to write output shares of the game results.");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  fbpcf::AwsSdk::aquire();

  XLOGF(INFO, "Party: {}", FLAGS_party);
  XLOGF(INFO, "Server IP: {}", FLAGS_server_ip);
  XLOGF(INFO, "Port: {}", FLAGS_port);
  XLOGF(INFO, "Input data path: {}", FLAGS_input_path);
  XLOGF(INFO, "Input params path: {}", FLAGS_input_params);
  XLOGF(INFO, "Output shares path: {}", FLAGS_output_file_path);

  if (FLAGS_party == fbpcf::edit_distance::PLAYER0) {
    XLOG(INFO, "Starting EditDistance as player 0, will wait for player 1");
    fbpcf::edit_distance::startEditDistanceGame<fbpcf::edit_distance::PLAYER0>(
        FLAGS_server_ip,
        FLAGS_port,
        FLAGS_input_path,
        FLAGS_input_params,
        FLAGS_output_file_path);

  } else if (FLAGS_party == fbpcf::edit_distance::PLAYER1) {
    XLOG(INFO, "Starting EditDistance as player 1, will wait for player 0");
    fbpcf::edit_distance::startEditDistanceGame<fbpcf::edit_distance::PLAYER1>(
        FLAGS_server_ip,
        FLAGS_port,
        FLAGS_input_path,
        FLAGS_input_params,
        FLAGS_output_file_path);
  } else {
    XLOGF(FATAL, "Invalid Party: {}", FLAGS_party);
  }

  return 0;
}
