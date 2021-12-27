/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ExceptionBase.h"

namespace fbpcf {
const char* ExceptionBase::what() const noexcept {
  return error_.c_str();
}
} // namespace fbpcf
