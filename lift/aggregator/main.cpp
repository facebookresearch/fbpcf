/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>

#include <gflags/gflags.h>

#include "folly/init/Init.h"
#include "folly/logging/xlog.h"

#include "../../pcf/aws/AwsSdk.h"
#include "../../pcf/exception/ExceptionBase.h"
#include "AggregatorApp.h"

DEFINE_int32(role, 1, "1 = publisher, 2 = partner");
DEFINE_int32(visibility, 0, "0 = public, 1 = publisher, 2 = partner");
DEFINE_string(server_ip, "", "Server's IP address");
DEFINE_int32(port, 15200, "Server's port");
DEFINE_string(input_path, "", "Input path where input files are located");
DEFINE_int32(
    num_shards,
    1,
    "Number of shards from input_path_[0] to input_path_[n-1]");
DEFINE_string(output_path, "", "Output path where output file is located");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  pcf::AwsSdk::aquire();

  XLOGF(INFO, "Role: {}", FLAGS_role);
  XLOGF(INFO, "Visibility: {}", FLAGS_visibility);
  XLOGF(INFO, "Server IP: {}", FLAGS_server_ip);
  XLOGF(INFO, "Port: {}", FLAGS_port);
  XLOGF(INFO, "Input path: {}", FLAGS_input_path);
  XLOGF(INFO, "Number of shards: {}", FLAGS_num_shards);
  XLOGF(INFO, "Output path: {}", FLAGS_output_path);

  XLOG(INFO) << "Start aggregating...";

  auto role = static_cast<pcf::Party>(FLAGS_role);
  auto visibility = static_cast<pcf::Visibility>(FLAGS_visibility);

  try {
    private_lift::AggregatorApp(
        role,
        visibility,
        FLAGS_server_ip,
        FLAGS_port,
        FLAGS_num_shards,
        FLAGS_input_path,
        FLAGS_output_path)
        .run();
  } catch (const pcf::ExceptionBase& e) {
    XLOGF(ERR, "Some error occured: {}", e.what());
    return 1;
  } catch (const std::exception& e) {
    XLOGF(ERR, "Some unknown error occured: {}", e.what());
    return -1;
  }

  XLOGF(
      INFO,
      "Aggregation is completed. Please find the metrics at {}",
      FLAGS_output_path);
  return 0;
}
