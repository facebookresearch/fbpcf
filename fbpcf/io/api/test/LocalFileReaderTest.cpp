/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include "fbpcf/io/api/IReaderCloser.h"

#include "fbpcf/io/api/FileReader.h"
#include "fbpcf/io/api/LocalFileReader.h"
#include "fbpcf/io/api/test/utils/IOTestHelper.h"

namespace fbpcf::io {

inline void runBaseReaderTests(IReaderCloser& reader) {
  EXPECT_FALSE(reader.eof());

  /*
    CASE 1A
    Buffer of size 20, read 20 bytes
  */
  auto buf = std::vector<char>(20);
  auto nBytes = reader.read(buf);

  EXPECT_EQ(nBytes, 20);
  IOTestHelper::expectBufferToEqualString(buf, "this is a test file\n", 20);
  EXPECT_FALSE(reader.eof());

  /*
      CASE 1B
      Buffer of size 25, read 25 bytes
  */
  auto buf2 = std::vector<char>(25);
  nBytes = reader.read(buf2);
  EXPECT_EQ(nBytes, 25);
  IOTestHelper::expectBufferToEqualString(
      buf2, "it has many lines in it\n\n", 25);
  EXPECT_FALSE(reader.eof());

  /*
      CASE 2
      Buffer of size 500, but file only has 45 bytes
      Expect to read 45 bytes
  */
  auto buf3 = std::vector<char>(500);

  nBytes = reader.read(buf3);

  EXPECT_EQ(nBytes, 45);
  IOTestHelper::expectBufferToEqualString(
      buf3, "the quick brown fox jumped over the lazy dog\n", 45);
  EXPECT_TRUE(reader.eof());
  EXPECT_THROW(reader.read(buf3), std::runtime_error);

  reader.close();
}

TEST(LocalFileReaderTest, testReadingFromFile) {
  auto reader = fbpcf::io::LocalFileReader(
      IOTestHelper::getBaseDirFromPath(__FILE__) +
      "data/local_file_reader_test_file.txt");

  runBaseReaderTests(reader);
}

TEST(LocalFileReaderTest, testLocalFileReaderThroughFileReader) {
  auto reader = fbpcf::io::FileReader(
      IOTestHelper::getBaseDirFromPath(__FILE__) +
      "data/local_file_reader_test_file.txt");

  runBaseReaderTests(reader);
}

} // namespace fbpcf::io
