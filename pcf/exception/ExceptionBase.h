#pragma once

#include <exception>
#include <string>

namespace pcf {
class ExceptionBase : public std::exception {
 public:
  explicit ExceptionBase(const std::string& error) : error_{error} {}
  explicit ExceptionBase(const std::exception& exception) : error_{exception.what()} {}

  const char* what() const noexcept override;

 protected:
  std::string error_;
};
} // namespace pcf
