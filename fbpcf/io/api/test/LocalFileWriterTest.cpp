/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <filesystem>
#include <random>
#include <string>
#include "folly/logging/xlog.h"

#include "fbpcf/io/api/FileWriter.h"
#include "fbpcf/io/api/LocalFileWriter.h"
#include "fbpcf/io/api/test/utils/IOTestHelper.h"

namespace fbpcf::io {

inline void runBaseWriterTests(
    IWriterCloser& writer,
    std::string fileToWriteTo,
    std::string expectedFile) {
  /*
    CASE 1
    Write simple string to file
  */
  std::string toWrite =
      "this file contains the expected text in local_file_writer_test_file.text";
  auto buf =
      std::vector<char>(toWrite.c_str(), toWrite.c_str() + toWrite.size());
  auto nBytes = writer.write(buf);
  EXPECT_EQ(nBytes, toWrite.size());

  /*
      CASE 2
      Write arbitrary bytes to file
  */
  std::vector<char> arbitraryBytes{'\n', '\n', 'L', 'o', 'c', 'a', 'l', 'F',
                                   'i',  'l',  'e', 'W', 'r', 'i', 't', 'e',
                                   'r',  'T',  'e', 's', 't', ' '};
  nBytes = writer.write(arbitraryBytes);
  EXPECT_EQ(nBytes, arbitraryBytes.size());

  /*
    CASE 3
    Write larger buffer
  */
  std::string remainingLine =
      "writes to the above file\nWe assert that it's contents match this file\n";
  auto buf2 = std::vector<char>(
      remainingLine.c_str(), remainingLine.c_str() + remainingLine.size());
  nBytes = writer.write(buf2);
  EXPECT_EQ(nBytes, remainingLine.size());

  writer.close();

  /*
    Verify that file contents match the expected
  */
  IOTestHelper::expectFileContentsMatch(fileToWriteTo, expectedFile);

  IOTestHelper::cleanup(fileToWriteTo);
}

TEST(LocalFileWriterTest, testWritingToFile) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::random_device rd;
  std::default_random_engine defEngine(rd());
  std::uniform_int_distribution<int> intDistro(1, 25000);
  auto randint = intDistro(defEngine);
  std::string fileToWriteTo = baseDir + "data/local_file_writer_test_file" +
      std::to_string(randint) + ".txt";
  std::string expectedFile =
      baseDir + "data/expected_local_file_writer_test_file.txt";

  auto writer = fbpcf::io::LocalFileWriter(fileToWriteTo);
  runBaseWriterTests(writer, fileToWriteTo, expectedFile);
}

TEST(LocalFileWriterTest, testLocalFileWriterThroughFileWriter) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::random_device rd;
  std::default_random_engine defEngine(rd());
  std::uniform_int_distribution<int> intDistro(1, 25000);
  auto randint = intDistro(defEngine);
  std::string fileToWriteTo = baseDir + "data/local_file_writer_test_file" +
      std::to_string(randint) + ".txt";
  std::string expectedFile =
      baseDir + "data/expected_local_file_writer_test_file.txt";

  auto writer = fbpcf::io::FileWriter(fileToWriteTo);
  runBaseWriterTests(writer, fileToWriteTo, expectedFile);
}

TEST(LocalFileWriterTest, testLocalFileWriterWithMissingDirectory) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::random_device rd;
  std::default_random_engine defEngine(rd());
  std::uniform_int_distribution<int> intDistro(1, 25000);
  auto randint = intDistro(defEngine);
  std::string fileToWriteTo = baseDir + "data/" + std::to_string(randint) +
      "/local_file_writer_test_file" + std::to_string(randint) + ".txt";
  std::string expectedFile =
      baseDir + "data/expected_local_file_writer_test_file.txt";

  auto writer = fbpcf::io::FileWriter(fileToWriteTo);
  EXPECT_NO_THROW(runBaseWriterTests(writer, fileToWriteTo, expectedFile));

  std::filesystem::remove("data/" + std::to_string(randint));
}

} // namespace fbpcf::io
