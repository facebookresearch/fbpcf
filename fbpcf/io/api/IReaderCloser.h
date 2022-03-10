/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/io/api/ICloser.h"
#include "fbpcf/io/api/IReader.h"

namespace fbpcf::io {

/*
 * Defines a class that reads data from an
 * underlying medium and closes it.
 */
class IReaderCloser : public IReader, public ICloser {
 public:
  virtual ~IReaderCloser() = default;
};

} // namespace fbpcf::io
