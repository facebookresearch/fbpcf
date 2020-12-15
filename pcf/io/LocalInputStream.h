#pragma once

#include <fstream>
#include <memory>

#include "IInputStream.h"

namespace pcf {
class LocalInputStream : public IInputStream {
 public:
  explicit LocalInputStream(std::ifstream is) : is_{std::move(is)} {}

  std::istream& get() override;

 private:
  std::ifstream is_;
};
} // namespace pcf
