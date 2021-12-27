/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "QueueIO.h"

namespace fbpcf {
void QueueIO::send_data_internal(const void* data, int64_t len) {
  outQueue_->withWLock([&data, len](auto& locked) {
    for (auto i = 0; i < len; i++) {
      locked.push(*((char*)data + i));
    }
  });
}

void QueueIO::recv_data_internal(void* data, int64_t len) {
  for (auto i = 0; i < len; i++) {
    while (inQueue_->rlock()->empty()) {
    }
    *((char*)data + i) = inQueue_->rlock()->front();
    inQueue_->wlock()->pop();
  }
}
} // namespace fbpcf
