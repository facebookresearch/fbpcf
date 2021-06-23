/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <gmock/gmock.h>

#include "IFileManager.h"

namespace fbpcf {
class MockFileManager : public IFileManager {
 public:
  MOCK_METHOD1(read, std::string(const std::string& fileName));

  MOCK_METHOD2(
      write,
      void(const std::string& fileName, const std::string& data));
};
} // namespace fbpcf
