/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include <cstdio>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "folly/Format.h"
#include "folly/Random.h"

#include "../../exception/PcfException.h"
#include "../LocalFileManager.h"

namespace pcf {
class LocalFileManagerTest : public ::testing::Test {
 protected:
  void TearDown() override {
    std::remove(filePath_.c_str());
  }

  const std::string testData_ = "this is test data";
  const std::string filePath_ =
      folly::sformat("./testfile_{}", folly::Random::rand32());
};

TEST_F(LocalFileManagerTest, testReadWrite) {
  LocalFileManager fileManager;
  fileManager.write(filePath_, testData_);
  auto resp = fileManager.read(filePath_);
  EXPECT_EQ(testData_, resp);
}

TEST_F(LocalFileManagerTest, testReadException) {
  LocalFileManager fileManager;
  EXPECT_THROW(fileManager.read("./fakedfile"), PcfException);
}
} // namespace pcf
