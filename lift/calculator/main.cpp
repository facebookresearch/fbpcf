/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdio>
#include <cstdlib>

#include <glog/logging.h>
#include <filesystem>
#include <sstream>
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
    input_directory,
    "",
    "Data directory where input files are located");
DEFINE_string(
    input_filenames,
    "in.csv_0[,in.csv_1,in.csv_2,...]",
    "List of input file names that should be parsed (should have a header)");
DEFINE_string(
    output_directory,
    "",
    "Local or s3 path where output files are written to");
DEFINE_string(output_filenames,
    "out.csv_0[,out.csv_1,out.csv_2,...]",
    "List of output file names that correspond to input filenames (positionally)");
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

using namespace private_lift;

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  pcf::AwsSdk::aquire();
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::filesystem::path inputDirectory{FLAGS_input_directory};
  std::filesystem::path outputDirectory{FLAGS_output_directory};

  std::vector<std::string> inputFilenames;
  folly::split(',', FLAGS_input_filenames, inputFilenames);

  std::vector<std::string> outputFilenames;
  folly::split(",", FLAGS_output_filenames, outputFilenames);

  // Make sure the number of input files equals output files
  if(inputFilenames.size() != outputFilenames.size()) {
    XLOGF(ERR, "Error: input_filenames items ({}) does not equal output_filenames items ({})", inputFilenames.size(), outputFilenames.size());
    return 1;
  }

  {
    // Build a quick list of input/output files to log
    std::ostringstream inputFileLogList;
    for (auto inputFilename : inputFilenames) {
      inputFileLogList << "\t\t" << inputFilename << "\n";
    }
    std::ostringstream outputFileLogList;
    for (auto outputFilename : outputFilenames) {
      outputFileLogList << "\t\t" << outputFilename << "\n";
    }
    XLOG(INFO) << "Running conversion lift with settings:\n"
               << "\trole: " << FLAGS_role << "\n"
               << "\tserver_ip_address: " << FLAGS_server_ip << "\n"
               << "\tport: " << FLAGS_port << "\n"
               << "\tconcurrency: " << FLAGS_concurrency << "\n"
               << "\tinput: " << inputDirectory << "\n"
               << inputFileLogList.str()
               << "\toutput: " << outputDirectory << "\n"
               << outputFileLogList.str();
  }

  auto role = static_cast<pcf::Party>(FLAGS_role);

  // since DEFINE_INT16 is not supported, cast int32_t FLAGS_concurrency to
  // int16_t is necessarys here
  int16_t concurrency = static_cast<int16_t>(FLAGS_concurrency);

  // construct calculatorApps according to  FLAGS_num_shards and
  // FLAGS_concurrency
  std::vector<std::unique_ptr<CalculatorApp>> calculatorApps;
  for (auto i = 0; i < inputFilenames.size(); i++) {
    calculatorApps.push_back(std::make_unique<CalculatorApp>(
        role,
        FLAGS_server_ip,
        FLAGS_port + i % concurrency,
        inputDirectory / inputFilenames[i],
        outputDirectory / outputFilenames[i],
        FLAGS_use_xor_encryption));
  }

  // executor calculatorApps using pcf::MpcAppExecutor
  pcf::MpcAppExecutor<CalculatorApp> executor{concurrency};
  executor.execute(calculatorApps);

  return 0;
}
