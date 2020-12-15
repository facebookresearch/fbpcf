#pragma once

#include <istream>

namespace pcf {
class IInputStream {
 public:
  virtual ~IInputStream() {}

  virtual std::istream& get() = 0;
};
} // namespace pcf
