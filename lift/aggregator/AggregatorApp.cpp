#include "AggregatorApp.h"

#include <memory>
#include <string>
#include <vector>

#include <emp-sh2pc/emp-sh2pc.h>

#include <folly/Format.h>
#include <folly/logging/xlog.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/io/FileManagerUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../common/GroupedLiftMetrics.h"
#include "LiftAggregationGame.h"

namespace private_lift {
void AggregatorApp::run() {
  auto io = std::make_unique<emp::NetIO>(
      party_ == pcf::Party::Alice ? nullptr : serverIp_.c_str(),
      port_,
      true /* quiet mode */);

  XLOG(INFO) << "NetIO is connected.";
  auto inputData = getInputData();

  LiftAggregationGame game{std::move(io), party_, visibility_};
  auto output = game.perfPlay(inputData);

  putOutputData(output);
};

std::vector<std::string> AggregatorApp::getInputPaths(
    const std::string& inputPath,
    const int numShards) {
  std::vector<std::string> v;

  for (int i = 0; i < numShards; i++) {
    v.push_back(folly::sformat("{}_{}", inputPath, i));
  }

  return v;
}

std::vector<GroupedLiftMetrics> AggregatorApp::getInputData() {
  XLOG(INFO) << "getting input data ...";
  auto inputPaths = AggregatorApp::getInputPaths(inputPath_, numShards_);
  return pcf::functional::map<std::string, GroupedLiftMetrics>(
      inputPaths, [](const auto& inputPath) {
        return GroupedLiftMetrics::fromJson(pcf::io::read(inputPath));
      });
}

void AggregatorApp::putOutputData(const GroupedLiftMetrics& metrics) {
  XLOG(INFO) << "putting out data ...";
  pcf::io::write(outputPath_, metrics.toJson());
}
} // namespace private_lift
