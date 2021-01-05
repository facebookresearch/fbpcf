/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "ExceptionBase.h"

namespace pcf {
class PcfException : public ExceptionBase {
 public:
  explicit PcfException(const std::string& error) : ExceptionBase{error} {}
  explicit PcfException(const std::exception& exception) : ExceptionBase{exception} {}
};
} // namespace pcf
