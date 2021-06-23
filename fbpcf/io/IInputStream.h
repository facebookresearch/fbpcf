/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <istream>

namespace fbpcf {
class IInputStream {
 public:
  virtual ~IInputStream() {}

  virtual std::istream& get() = 0;
};
} // namespace fbpcf
