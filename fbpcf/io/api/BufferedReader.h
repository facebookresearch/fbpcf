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

#include "fbpcf/io/api/IOUtils.h"
#include "fbpcf/io/api/IReaderCloser.h"

namespace fbpcf::io {

/*
This class is the API for buffered reading, which
provides a readLine function as well as the ability
to specify a chunk size.
*/

constexpr size_t defaultReaderChunkSize = 4096;

class BufferedReader : public IReaderCloser {
 public:
  explicit BufferedReader(
      std::unique_ptr<IReaderCloser> baseReader,
      const size_t chunkSize)
      : buffer_{std::vector<char>(chunkSize)},
        currentPosition_{0},
        lastPosition_{0} {
    filepath_ = baseReader->getFilePath();
    baseReader_ = std::move(baseReader);
  }

  explicit BufferedReader(std::unique_ptr<IReaderCloser> baseReader)
      : currentPosition_{0}, lastPosition_{0} {
    auto defaultChunkSizeForFile =
        IOUtils::getDefaultReaderChunkSizeForFile(baseReader->getFilePath());
    filepath_ = baseReader->getFilePath();
    buffer_ = std::vector<char>(defaultChunkSizeForFile);
    baseReader_ = std::move(baseReader);
  }

  int close() override;
  size_t read(std::vector<char>& buf) override;
  bool eof() override;
  ~BufferedReader() override;

  std::string readLine();

 private:
  void loadNextChunk();

  std::vector<char> buffer_;
  size_t currentPosition_;
  std::unique_ptr<IReaderCloser> baseReader_;
  size_t lastPosition_;
};

} // namespace fbpcf::io
