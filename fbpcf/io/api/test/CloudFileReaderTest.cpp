/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "fbpcf/io/api/CloudFileReader.h"
#include "fbpcf/io/cloud_util/IFileReader.h"

namespace fbpcf::io {

class MockFileReader : public fbpcf::cloudio::IFileReader {
 public:
  MOCK_METHOD(
      std::string,
      readBytes,
      (const std::string&, std::size_t, std::size_t));
  MOCK_METHOD(size_t, getFileContentLength, (const std::string&));
};

class CloudFileReaderTest : public ::testing::Test {
 protected:
  void TearDown() override {
    fileReader_ = nullptr;
  }

  void SetUp() override {
    fileReader_ = std::make_shared<MockFileReader>();
  }

  std::shared_ptr<MockFileReader> fileReader_;
};

TEST_F(CloudFileReaderTest, testRead) {
  const std::string contents = "fileContents";

  EXPECT_CALL(*fileReader_, getFileContentLength(testing::_))
      .Times(testing::AnyNumber())
      .WillRepeatedly(testing::Return(contents.length()));

  EXPECT_CALL(*fileReader_, readBytes(testing::_, testing::_, testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(contents));

  auto cloudFileReader = std::unique_ptr<fbpcf::io::CloudFileReader>(
      new fbpcf::io::CloudFileReader("s3://bkt/file", fileReader_));

  std::vector<char> buf;
  auto bytesCnt = cloudFileReader->read(buf);

  EXPECT_EQ(contents.length(), bytesCnt);
  EXPECT_EQ(contents, std::string(buf.begin(), buf.end()));
}

} // namespace fbpcf::io
