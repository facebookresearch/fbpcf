/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::io {

/*
 * Defines a class that uses an underlying
 * medium and must close it to free up the allocated
 * resources.
 */
class ICloser {
 public:
  /*
   * close() returns 0 if it succeeds, and -1
   * in the case of an error.
   */
  virtual int close();
  virtual ~ICloser();
};

} // namespace fbpcf::io
