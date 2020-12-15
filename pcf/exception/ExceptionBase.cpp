#include "ExceptionBase.h"

namespace pcf {
const char* ExceptionBase::what() const noexcept {
  return error_.c_str();
}
} // namespace pcf
