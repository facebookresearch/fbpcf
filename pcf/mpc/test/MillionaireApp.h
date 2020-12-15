
#pragma once

#include <emp-sh2pc/emp-sh2pc.h>

#include "../EmpApp.h"
#include "./MillionaireGame.h"

namespace pcf {
class MillionaireApp : public EmpApp<MillionaireGame<emp::NetIO>, int, bool> {
 public:
  MillionaireApp(Party party, const std::string& serverIp, uint16_t port)
      : EmpApp<MillionaireGame<emp::NetIO>, int, bool>{party, serverIp, port} {}

 protected:
  int getInputData() override;
  void putOutputData(const bool& output) override;
};
} // namespace pcf
