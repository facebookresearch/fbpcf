
#include "folly/Random.h"

#include "./MillionaireApp.h"

namespace pcf {

int MillionaireApp::getInputData() {
  return folly::Random::rand32();
}

void MillionaireApp::putOutputData(const bool&) {}
} // namespace pcf
