#include "S3InputStream.h"

#include <istream>

namespace pcf {
std::istream& S3InputStream::get() {
  return r_.GetBody();
}
} // namespace pcf
