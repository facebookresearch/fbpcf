/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <filesystem>
#include <string>
#include "folly/logging/xlog.h"

#include "fbpcf/io/api/BufferedWriter.h"
#include "fbpcf/io/api/LocalFileWriter.h"
#include "fbpcf/io/api/test/utils/IOTestHelper.h"

namespace fbpcf::io {

TEST(BufferedWriterTest, testWritingToFile) {
  std::string base_dir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::string file_to_write_to =
      base_dir + "data/buffered_writer_writer_test_file.txt";
  auto writer = fbpcf::io::LocalFileWriter(file_to_write_to);
  auto bufferedWriter = std::make_unique<BufferedWriter>(writer, 40);

  std::string to_write = "this file tests the buffered writer\n";
  auto buf =
      std::vector<char>(to_write.c_str(), to_write.c_str() + to_write.size());
  auto nBytes = bufferedWriter->write(buf);
  EXPECT_EQ(nBytes, to_write.size());

  std::vector<char> arbitraryBytes{
      't', 'h', 'e', 's', 'e', ' ', 't', 'w', 'o', ' '};
  nBytes = bufferedWriter->write(arbitraryBytes);
  EXPECT_EQ(nBytes, arbitraryBytes.size());

  to_write = "lines fit in one chunk\n";
  auto buf2 =
      std::vector<char>(to_write.c_str(), to_write.c_str() + to_write.size());
  nBytes = bufferedWriter->write(buf2);
  EXPECT_EQ(nBytes, to_write.size());

  std::string longLine =
      "but this next line is going to be much longer and will require multiple iterations of the loop to fit everything in the file\n";
  auto buf3 =
      std::vector<char>(longLine.c_str(), longLine.c_str() + longLine.size());
  nBytes = bufferedWriter->write(buf3);
  EXPECT_EQ(nBytes, longLine.size());
  bufferedWriter->flush();

  to_write = "this is tiny\n";
  auto buf4 =
      std::vector<char>(to_write.c_str(), to_write.c_str() + to_write.size());
  nBytes = bufferedWriter->write(buf4);
  EXPECT_EQ(nBytes, to_write.size());

  bufferedWriter->close();

  /*
    Verify that file contents match the expected
  */
  IOTestHelper::expectFileContentsMatch(
      file_to_write_to,
      base_dir + "data/expected_buffered_writer_test_file.txt");

  IOTestHelper::cleanup(file_to_write_to);
}

} // namespace fbpcf::io
