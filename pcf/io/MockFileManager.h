#pragma once

#include <gmock/gmock.h>

#include "IFileManager.h"

namespace pcf {
class MockFileManager : public IFileManager {
 public:
  MOCK_METHOD1(read, std::string(const std::string& fileName));

  MOCK_METHOD2(
      write,
      void(const std::string& fileName, const std::string& data));
};
} // namespace pcf
