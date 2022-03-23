/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <memory>

#include "fbpcf/io/api/BufferedReader.h"
#include "fbpcf/io/api/FileReader.h"
#include "fbpcf/io/api/test/utils/IOTestHelper.h"

namespace fbpcf::io {

TEST(BufferedReaderTest, testBufferedReaderWithBothReadAndReadLine) {
  auto fileReader = fbpcf::io::FileReader(
      IOTestHelper::getBaseDirFromPath(__FILE__) +
      "data/buffered_reader_test_file.txt");
  auto bufferedReader =
      std::make_unique<fbpcf::io::BufferedReader>(fileReader, 40);

  auto firstLine = bufferedReader->readLine();

  EXPECT_EQ(firstLine, "this is a simple first line");
  EXPECT_FALSE(bufferedReader->eof());

  auto secondLine = bufferedReader->readLine();

  EXPECT_EQ(
      secondLine,
      "this is a second line that is intended to be longer than a line that can fit in a single buffer");
  EXPECT_FALSE(bufferedReader->eof());

  auto thirdLine = bufferedReader->readLine();
  std::string expectedLongString =
      "this is a third line that should take at least three iterations of the buffer so that we can properly check the iterative functionality";
  EXPECT_EQ(thirdLine, expectedLongString);
  EXPECT_FALSE(bufferedReader->eof());

  auto fourthLine = bufferedReader->readLine();
  EXPECT_EQ(fourthLine, "");
  EXPECT_FALSE(bufferedReader->eof());

  auto blurb = std::vector<char>(65);
  auto nBytes = bufferedReader->read(blurb);
  EXPECT_EQ(nBytes, 65);
  IOTestHelper::expectBufferToEqualString(
      blurb,
      "we also test a blank line and\na line that will include a newline\n",
      65);
  EXPECT_FALSE(bufferedReader->eof());

  auto blankLine = bufferedReader->readLine();
  EXPECT_EQ(blankLine, "");
  EXPECT_FALSE(bufferedReader->eof());

  auto remainingBytes = std::vector<char>(300);
  nBytes = bufferedReader->read(remainingBytes);
  EXPECT_EQ(nBytes, 242);
  std::string expectedLongParagraph =
      "finally, we want to test a really long block of\ntext, that will take 3 or 4 chunks, but this time\nusing the read API instead of the readLine API to\nmake sure that we have adequate test coverage. the\ntotal size of this paragraph is 242 bytes.\n";
  IOTestHelper::expectBufferToEqualString(
      remainingBytes, expectedLongParagraph, nBytes);
  EXPECT_TRUE(bufferedReader->eof());

  bufferedReader->close();
}

TEST(BufferedReaderTest, testBufferedReaderWithReadLineOnly) {
  // this more accurately resembles a production style usage
  auto fileReader = fbpcf::io::FileReader(
      IOTestHelper::getBaseDirFromPath(__FILE__) +
      "data/buffered_reader_test_file.txt");
  auto bufferedReader =
      std::make_unique<fbpcf::io::BufferedReader>(fileReader, 40);

  auto expectedLines = std::vector<std::string>{
      "this is a simple first line",
      "this is a second line that is intended to be longer than a line that can fit in a single buffer",
      "this is a third line that should take at least three iterations of the buffer so that we can properly check the iterative functionality",
      "",
      "we also test a blank line and",
      "a line that will include a newline",
      "",
      "finally, we want to test a really long block of",
      "text, that will take 3 or 4 chunks, but this time",
      "using the read API instead of the readLine API to",
      "make sure that we have adequate test coverage. the",
      "total size of this paragraph is 242 bytes."};

  auto i = 0;
  while (!bufferedReader->eof()) {
    auto line = bufferedReader->readLine();
    EXPECT_EQ(line, expectedLines.at(i));
    i++;

    if (i == expectedLines.size()) {
      EXPECT_TRUE(bufferedReader->eof());
    } else {
      EXPECT_FALSE(bufferedReader->eof());
    }
  }
}

} // namespace fbpcf::io
