/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <queue>

#include <emp-sh2pc/emp-sh2pc.h>

#include "folly/Synchronized.h"

namespace fbpcf {
class QueueIO : public emp::IOChannel<QueueIO> {
 public:
  QueueIO(
      const std::shared_ptr<folly::Synchronized<std::queue<char>>> inQueue,
      const std::shared_ptr<folly::Synchronized<std::queue<char>>> outQueue)
      : inQueue_{inQueue}, outQueue_{outQueue} {}

  void send_data_internal(const void* data, int64_t len);
  void recv_data_internal(void* data, int64_t len);

  /* This is a design issue in EMP. flush() is not part of IOChannel interface,
   * but it's coupled in implementation. EMP should either defines it in
   * IOChannel, or the implementation shouldn't assume flush() is in the
   * implementation. */
  void flush() {}

 private:
  const std::shared_ptr<folly::Synchronized<std::queue<char>>> inQueue_;
  const std::shared_ptr<folly::Synchronized<std::queue<char>>> outQueue_;
};
} // namespace fbpcf
