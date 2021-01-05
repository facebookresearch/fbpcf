/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>

#include <glog/logging.h>
#include <filesystem>
#include <string>

#include <gflags/gflags.h>

#include <folly/init/Init.h>
#include "folly/logging/xlog.h"

#include "../../pcf/aws/AwsSdk.h"
#include "../../pcf/mpc/experimental/MpcAppExecutor.h"
#include "CalculatorApp.h"

DEFINE_int32(role, 1, "1 = publisher, 2 = partner");
DEFINE_string(server_ip, "127.0.0.1", "Server's IP Address");
DEFINE_int32(
    port,
    5000,
    "Network port for establishing connection to other player");
DEFINE_string(
    data_directory,
    "",
    "Data directory where input files are located");
DEFINE_string(
    input_filename,
    "in.csv",
    "Name of the input file that should be parsed (should have a header)");
DEFINE_string(output_path, "", "local or s3 path of output files");
DEFINE_int64(
    epoch,
    1546300800,
    "Unixtime of 2019-01-01. Used as our 'new epoch' for timestamps");
DEFINE_bool(
    is_conversion_lift,
    true,
    "Use conversion_lift logic (as opposed to converter_lift logic)");
DEFINE_bool(
    use_xor_encryption,
    true,
    "Reveal output with XOR secret shares instead of in the clear to both parties");
DEFINE_int32(
    num_conversions_per_user,
    4,
    "Cap and pad to this many conversions per user");
DEFINE_int32(
    concurrency,
    1,
    "max number of game(s) that will run concurrently?");
DEFINE_int32(num_shards, 1, "total number of games to run");

using namespace private_lift;

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  pcf::AwsSdk::aquire();
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::filesystem::path baseDirectory{FLAGS_data_directory};

  auto inputPath = baseDirectory / FLAGS_input_filename;

  XLOG(INFO) << "Running conversion lift with settings:\n"
             << "\trole: " << FLAGS_role << "\n"
             << "\tserver_ip_address: " << FLAGS_server_ip << "\n"
             << "\tport: " << FLAGS_port << "\n"
             << "\tnum_shards: " << FLAGS_num_shards << "\n"
             << "\tconcurrency: " << FLAGS_concurrency << "\n"
             << "\tinputPath: " << inputPath << "\n";

  auto role = static_cast<pcf::Party>(FLAGS_role);

  // since DEFINE_INT16 is not supported, cast int32_t FLAGS_concurrency to
  // int16_t is necessarys here
  int16_t concurrency = static_cast<int16_t>(FLAGS_concurrency);

  // construct calculatorApps according to  FLAGS_num_shards and
  // FLAGS_concurrency
  std::vector<std::unique_ptr<CalculatorApp>> calculatorApps;
  for (auto i = 0; i < FLAGS_num_shards; i++) {
    calculatorApps.push_back(std::make_unique<CalculatorApp>(
        role,
        FLAGS_server_ip,
        FLAGS_port + i % concurrency,
        // given input_path, the ith task takes file input_path_i
        baseDirectory / (FLAGS_input_filename + "_" + std::to_string(i)),
        // given output_path, the ith task outputs output file output_path_i
        FLAGS_output_path + "_" + std::to_string(i),
        FLAGS_use_xor_encryption));
  }

  // executor calculatorApps using pcf::MpcAppExecutor
  pcf::MpcAppExecutor<CalculatorApp> executor{concurrency};
  executor.execute(calculatorApps);

  return 0;
}
