#include "QueueIO.h"

namespace pcf {
void QueueIO::send_data(const void* data, int len) {
  outQueue_->withWLock([&data, len](auto& locked) {
    for (auto i = 0; i < len; i++) {
      locked.push(*((char*)data + i));
    }
  });
}

void QueueIO::recv_data(void* data, int len) {
  for (auto i = 0; i < len; i++) {
    while (inQueue_->rlock()->empty()) {
    }
    *((char*)data + i) = inQueue_->rlock()->front();
    inQueue_->wlock()->pop();
  }
}
} // namespace pcf
