/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/scheduler/SchedulerHelper.h>
#include "./EditDistanceApp.h" // @manual

namespace fbpcf::edit_distance {

template <int schedulerId>
void EditDistanceApp<schedulerId>::run() {
  auto scheduler = fbpcf::scheduler::createLazySchedulerWithRealEngine(
      myRole_, *communicationAgentFactory_);

  XLOG(INFO, "Scheduler created successfully");
  auto editDistanceGame =
      EditDistanceGame<schedulerId>(myRole_, std::move(scheduler));

  EditDistanceInputReader inputReader(dataFilePath_, paramsFilePath_);

  std::string output = editDistanceGame.play(std::move(inputReader));

  XLOG(INFO) << "Writing results...";
  fbpcf::io::FileIOWrappers::writeFile(outFilePath_, output);
}
} // namespace fbpcf::edit_distance
