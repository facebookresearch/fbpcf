/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "LocalInputStream.h"

#include <istream>

namespace fbpcf {
std::istream& LocalInputStream::get() {
  return is_;
}
} // namespace fbpcf
