/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "S3InputStream.h"

#include <istream>

namespace fbpcf {
std::istream& S3InputStream::get() {
  return r_.GetBody();
}
} // namespace fbpcf
