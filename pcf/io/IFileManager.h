#pragma once

#include <istream>
#include <string>
#include <memory>

#include "IInputStream.h"

namespace pcf {
class IFileManager {
 public:
  virtual ~IFileManager() {}

  virtual std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) = 0;

  virtual std::string read(const std::string& fileName) = 0;

  virtual void write(const std::string& fileName, const std::string& data) = 0;
};
} // namespace pcf
