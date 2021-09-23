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

#include "fbpcf/exception/PcfException.h"
#include "fbpcf/io/LocalFileManager.h"

namespace fbpcf {
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

TEST_F(LocalFileManagerTest, testWriteException) {
  LocalFileManager fileManager;
  EXPECT_THROW(
      fileManager.write("./fakedfolder/fakedfile", testData_), PcfException);
}

TEST_F(LocalFileManagerTest, testWriteReadBytes) {
  LocalFileManager fileManager;
  fileManager.write(filePath_, testData_);
  auto resp1 = fileManager.readBytes(filePath_, 0, 5);
  EXPECT_EQ(resp1, "this ");

  auto resp2 = fileManager.readBytes(filePath_, 10, 15);
  EXPECT_EQ(resp2, "st da");

  auto resp3 = fileManager.readBytes(filePath_, 1, 1);
  EXPECT_EQ(resp3, "");

  auto resp4 = fileManager.readBytes(filePath_, 15, 20);
  EXPECT_EQ(resp4, "ta");
}

TEST_F(LocalFileManagerTest, testByteReadException) {
  LocalFileManager fileManager;
  fileManager.write(filePath_, testData_);
  EXPECT_THROW(fileManager.readBytes(filePath_, 100, 101), PcfException);
  EXPECT_THROW(fileManager.readBytes(filePath_, 5, 1), PcfException);
}
} // namespace fbpcf
