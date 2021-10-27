/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GCSInputStream.h"

#include <istream>

namespace fbpcf {
std::istream& GCSInputStream::get() {
  return s_;
}
} // namespace fbpcf
