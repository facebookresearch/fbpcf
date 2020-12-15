#pragma once

#include <memory>
#include <queue>

#include <emp-sh2pc/emp-sh2pc.h>

#include "folly/Synchronized.h"

namespace pcf {
class QueueIO : public emp::IOChannel<QueueIO> {
 public:
  QueueIO(
      const std::shared_ptr<folly::Synchronized<std::queue<char>>>
          inQueue,
      const std::shared_ptr<folly::Synchronized<std::queue<char>>>
          outQueue)
      : inQueue_{inQueue}, outQueue_{outQueue} {}

  void send_data(const void* data, int len);
  void recv_data(void* data, int len);

  /* This is a design issue in EMP. flush() is not part of IOChannel interface,
   * but it's coupled in implementation. EMP should either defines it in
   * IOChannel, or the implementation shouldn't assume flush() is in the
   * implementation. */
  void flush() {}

 private:
  const std::shared_ptr<folly::Synchronized<std::queue<char>>> inQueue_;
  const std::shared_ptr<folly::Synchronized<std::queue<char>>> outQueue_;
};
} // namespace pcf
