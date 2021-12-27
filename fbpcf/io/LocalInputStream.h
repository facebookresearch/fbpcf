/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fstream>
#include <memory>

#include "IInputStream.h"

namespace fbpcf {
class LocalInputStream : public IInputStream {
 public:
  explicit LocalInputStream(std::ifstream is) : is_{std::move(is)} {}

  std::istream& get() override;

 private:
  std::ifstream is_;
};
} // namespace fbpcf
