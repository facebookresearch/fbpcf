#pragma once

#include "ExceptionBase.h"

namespace pcf {
class PcfException : public ExceptionBase {
 public:
  explicit PcfException(const std::string& error) : ExceptionBase{error} {}
  explicit PcfException(const std::exception& exception) : ExceptionBase{exception} {}
};
} // namespace pcf
