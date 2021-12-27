/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <istream>

#include <aws/s3/model/GetObjectResult.h>

#include "IInputStream.h"

namespace fbpcf {
class S3InputStream : public IInputStream {
 public:
  explicit S3InputStream(Aws::S3::Model::GetObjectResult r)
      : r_{std::move(r)} {}

  std::istream& get() override;

 private:
  Aws::S3::Model::GetObjectResult r_;
};
} // namespace fbpcf
