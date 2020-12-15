#pragma once

#include <memory>

#include "IFileManager.h"

namespace pcf {
class LocalFileManager : public IFileManager {
 public:
  std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) override;

  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;
};
} // namespace pcf
