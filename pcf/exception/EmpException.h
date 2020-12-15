#pragma oncee

#include "ExceptionBase.h"

namespace pcf {
class EmpException : public ExceptionBase {
 public:
  explicit EmpException(const std::string& error) : ExceptionBase{error} {}
  explicit EmpException(const std::exception& exception) : ExceptionBase{exception} {}
};
} // namespace pcf
