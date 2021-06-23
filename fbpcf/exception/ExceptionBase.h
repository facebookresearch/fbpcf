/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <exception>
#include <string>

namespace fbpcf {
class ExceptionBase : public std::exception {
 public:
  explicit ExceptionBase(const std::string& error) : error_{error} {}
  explicit ExceptionBase(const std::exception& exception) : error_{exception.what()} {}

  const char* what() const noexcept override;

 protected:
  std::string error_;
};
} // namespace fbpcf
