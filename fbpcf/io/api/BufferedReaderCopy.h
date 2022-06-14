/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "fbpcf/io/api/IReaderCloser.h"

namespace fbpcf::io {

/*
This class is the API for buffered reading, which
provides a readLine function as well as the ability
to specify a chunk size.
*/

constexpr size_t defaultReaderChunkSize = 4096;

class BufferedReaderCopy : public IReaderCloser {
 public:
  explicit BufferedReaderCopy(
      IReaderCloser& baseReader,
      const size_t chunkSize = defaultReaderChunkSize)
      : buffer_{std::vector<char>(chunkSize)},
        currentPosition_{0},
        baseReader_{baseReader},
        lastPosition_{0} {}

  int close() override;
  size_t read(std::vector<char>& buf) override;
  bool eof() override;
  ~BufferedReaderCopy() override;

  std::string readLine();

 private:
  void loadNextChunk();

  std::vector<char> buffer_;
  size_t currentPosition_;
  IReaderCloser& baseReader_;
  size_t lastPosition_;
};

} // namespace fbpcf::io
