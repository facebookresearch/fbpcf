/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <future>
#include <vector>

namespace fbpcf::mpc_framework::engine::util {

/**
 * Holds a buffer that returns the requested amount of data on-demand. Data is
 * regenerated in chunks asynchronously.
 */
template <typename T>
class AsyncBuffer {
 public:
  AsyncBuffer(
      uint64_t bufferSize,
      std::function<std::vector<T>(uint64_t size)> generateData)
      : bufferSize_{bufferSize},
        bufferIndex_{bufferSize},
        generateData_{generateData} {
    futureBuffer_ = std::async(generateData_, bufferSize_);
  }

  ~AsyncBuffer() {
    futureBuffer_.get();
  }

  std::vector<T> getData(uint64_t size) {
    std::vector<T> rst;
    while (rst.size() < size) {
      if (bufferIndex_ >= bufferSize_) {
        buffer_ = futureBuffer_.get();
        bufferIndex_ = 0;
        futureBuffer_ = std::async(generateData_, bufferSize_);
      }

      auto insertSize = std::min(size - rst.size(), bufferSize_ - bufferIndex_);
      rst.insert(
          rst.end(),
          buffer_.begin() + bufferIndex_,
          buffer_.begin() + bufferIndex_ + insertSize);
      bufferIndex_ += insertSize;
    }
    return rst;
  }

 private:
  uint64_t bufferSize_;
  uint64_t bufferIndex_;

  std::function<std::vector<T>(uint64_t size)> generateData_;

  std::vector<T> buffer_;

  std::future<std::vector<T>> futureBuffer_;
};

} // namespace fbpcf::mpc_framework::engine::util
