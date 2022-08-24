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
#include "fbpcf/io/api/IWriterCloser.h"

namespace fbpcf::io {

/*
This class is the API for buffered writer, which
provides the ability to specify a chunk size.
*/

constexpr size_t defaultWriterChunkSize = 4096;

class BufferedWriter : public IWriterCloser {
 public:
  explicit BufferedWriter(
      std::unique_ptr<IWriterCloser> baseWriter,
      const size_t chunkSize)
      : buffer_{std::vector<char>(chunkSize)}, currentPosition_{0} {
    filepath_ = baseWriter->getFilePath();
    baseWriter_ = std::move(baseWriter);
  }

  explicit BufferedWriter(std::unique_ptr<IWriterCloser> baseWriter)
      : currentPosition_{0} {
    auto defaultChunkSizeForFile =
        IOUtils::getDefaultWriterChunkSizeForFile(baseWriter->getFilePath());
    filepath_ = baseWriter->getFilePath();
    baseWriter_ = std::move(baseWriter);
    buffer_ = std::vector<char>(defaultChunkSizeForFile);
  }

  int close() override;
  size_t write(std::vector<char>& buf) override;
  size_t writeString(const std::string& line);
  ~BufferedWriter() override;

  void flush();

 private:
  std::vector<char> buffer_;
  size_t currentPosition_;
  std::unique_ptr<IWriterCloser> baseWriter_;
};

} // namespace fbpcf::io
