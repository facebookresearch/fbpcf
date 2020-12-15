#include "LocalInputStream.h"

#include <istream>

namespace pcf {
std::istream& LocalInputStream::get() {
  return is_;
}
} // namespace pcf
