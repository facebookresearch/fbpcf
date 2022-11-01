/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>

#include <gflags/gflags.h>

#include "folly/init/Init.h"
#include "folly/logging/xlog.h"

#include "./MillionaireApp.h" // @manual
#include "fbpcf/exception/ExceptionBase.h"

DEFINE_int32(role, 1, "1 = Alice, 2 = Bob");
DEFINE_string(server_ip, "", "Server's IP address");
DEFINE_int32(port, 5000, "Server's port");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  XLOGF(INFO, "Role: {}", FLAGS_role);
  XLOGF(INFO, "Server IP: {}", FLAGS_server_ip);
  XLOGF(INFO, "Port: {}", FLAGS_port);

  XLOG(INFO) << "Start Millionaire Game...";

  auto role = static_cast<fbpcf::Party>(FLAGS_role);

  try {
    fbpcf::MillionaireApp(role, FLAGS_server_ip, FLAGS_port).run();
  } catch (const fbpcf::ExceptionBase& e) {
    XLOGF(ERR, "Some error occured: {}", e.what());
    return 1;
  } catch (const std::exception& e) {
    XLOGF(ERR, "Some unknown error occured: {}", e.what());
    return -1;
  }

  XLOG(INFO) << "Millionaire Game is completed.";
  return 0;
}
