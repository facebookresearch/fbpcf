#pragma once

#include "ExceptionBase.h"

namespace pcf {
class AwsException : public ExceptionBase {
 public:
  explicit AwsException(const std::string& error) : ExceptionBase{error} {}
  explicit AwsException(const std::exception& exception) : ExceptionBase{exception} {}
};
} // namespace pcf
