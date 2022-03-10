/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::io {

/*
 * Defines a class that reads data from an
 * underlying medium.
 */
class IReader {
 public:
  /*
   * read() returns the number of bytes it was
   * able to read, or -1 if error. It fills
   * the provided buffer with the data that was
   * read
   */
  virtual int read(char buf[]) = 0;
  virtual ~IReader() = default;
};

} // namespace fbpcf::io
