/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/FileIOWrappers.h"
#include <fbpcf/test/TestHelper.h>
#include <folly/Format.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <filesystem>
#include <string>
#include "fbpcf/io/api/test/utils/IOTestHelper.h"
#include "folly/Random.h"

namespace fbpcf::io {

TEST(FileIOWrappersTest, testTransferFileInParts) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::string fileToWriteTo = folly::sformat(
      "{}data/file_io_wrapper_transfer_in_parts_test_file_{}.txt",
      baseDir,
      folly::Random::secureRand64());
  std::string expectedFile =
      baseDir + "data/expected_file_io_wrapper_transfer_in_parts_test_file.txt";

  fbpcf::io::FileIOWrappers::transferFileInParts(expectedFile, fileToWriteTo);

  /*
    Verify that file contents match the expected
  */
  IOTestHelper::expectFileContentsMatch(fileToWriteTo, expectedFile);
  IOTestHelper::cleanup(fileToWriteTo);
}

TEST(FileIOWrappersTest, testWriteFile) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::string fileToWriteTo = folly::sformat(
      "{}data/file_io_wrapper_transfer_in_parts_test_file_{}.txt",
      baseDir,
      folly::Random::secureRand64());
  std::string fileToRead = baseDir + "data/file_io_wrappers_read_file_test.txt";

  std::string content =
      "this is a test file\n"
      "here's the second line, please read me\n"
      "\n"
      "last line here, don't miss me ~~\n";
  fbpcf::io::FileIOWrappers::writeFile(fileToWriteTo, content);

  IOTestHelper::expectFileContentsMatch(fileToWriteTo, fileToRead);
  IOTestHelper::cleanup(fileToWriteTo);
}

TEST(FileIOWrappersTest, testReadFile) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::string fileToRead = baseDir + "data/file_io_wrappers_read_file_test.txt";

  std::string content = fbpcf::io::FileIOWrappers::readFile(fileToRead);

  std::string expectedContent =
      "this is a test file\n"
      "here's the second line, please read me\n"
      "\n"
      "last line here, don't miss me ~~\n";
  EXPECT_EQ(content, expectedContent);
}

TEST(FileIOWrappersTest, testReadCSV) {
  std::string baseDir = IOTestHelper::getBaseDirFromPath(__FILE__);
  std::string fileToRead = baseDir + "data/file_io_wrappers_read_csv_test.csv";

  std::vector<std::vector<std::string>> csvContents;

  fbpcf::io::FileIOWrappers::readCsv(
      fileToRead,
      [&csvContents](
          const std::vector<std::string>& header,
          const std::vector<std::string>& values) {
        csvContents.push_back(values);
      });

  std::vector<std::vector<std::string>> expected = {
      {"Paul", "35", "Programmer"},
      {"", "13", ""},
      {""},
      {"Bill", "", "Lawyer"}};

  EXPECT_EQ(csvContents.size(), expected.size());

  for (int i = 0; i < csvContents.size(); i++) {
    testVectorEq(csvContents[i], expected[i]);
  }
}

} // namespace fbpcf::io
