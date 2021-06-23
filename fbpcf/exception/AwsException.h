/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "ExceptionBase.h"

namespace fbpcf {
class AwsException : public ExceptionBase {
 public:
  explicit AwsException(const std::string& error) : ExceptionBase{error} {}
  explicit AwsException(const std::exception& exception) : ExceptionBase{exception} {}
};
} // namespace fbpcf
