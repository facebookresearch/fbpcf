/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ExceptionBase.h"

namespace fbpcf {
class GcpException : public ExceptionBase {
 public:
  explicit GcpException(const std::string& error) : ExceptionBase{error} {}
  explicit GcpException(const std::exception& exception)
      : ExceptionBase{exception} {}
};
} // namespace fbpcf
