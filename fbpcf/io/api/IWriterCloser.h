/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/io/api/ICloser.h"
#include "fbpcf/io/api/IWriter.h"

namespace fbpcf::io {

/*
 * Defines a class that writes data to an
 * underlying medium and closes it.
 */
class IWriterCloser : public IWriter, public ICloser {
 public:
  virtual ~IWriterCloser() = default;
};

} // namespace fbpcf::io
