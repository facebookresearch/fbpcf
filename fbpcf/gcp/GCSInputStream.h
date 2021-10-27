/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <google/cloud/storage/object_read_stream.h>
#include <istream>

#include "fbpcf/io/IInputStream.h"
namespace gcs = ::google::cloud::storage;
namespace fbpcf {
class GCSInputStream : public IInputStream {
 public:
  explicit GCSInputStream(gcs::ObjectReadStream s) : s_{std::move(s)} {}

  std::istream& get() override;

 private:
  gcs::ObjectReadStream s_;
};
} // namespace fbpcf
