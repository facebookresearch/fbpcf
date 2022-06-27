/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>
#include <string>

#include "fbpcf/io/api/BufferedReader.h"
#include "fbpcf/io/api/BufferedWriter.h"
#include "fbpcf/io/api/FileIOWrappers.h"
#include "fbpcf/io/api/FileReader.h"
#include "fbpcf/io/api/FileWriter.h"

namespace fbpcf::io {

std::string FileIOWrappers::readFile(const std::string& srcPath) {
  auto reader = std::make_unique<fbpcf::io::FileReader>(srcPath);
  auto bufferedReader = std::make_unique<fbpcf::io::BufferedReader>(
      std::move(reader), kBufferedReaderChunkSize);
  std::string output = "";
  std::string newLine = "\n";
  while (!bufferedReader->eof()) {
    auto line = bufferedReader->readLine();
    output += line + newLine;
  }
  bufferedReader->close();
  return output;
}

void FileIOWrappers::writeFile(
    const std::string& destPath,
    const std::string& content) {
  auto fileWriter = std::make_unique<fbpcf::io::FileWriter>(destPath);
  auto bufferedWriter = std::make_unique<fbpcf::io::BufferedWriter>(
      std::move(fileWriter), kBufferedWriterChunkSize);
  bufferedWriter->writeString(content);
  bufferedWriter->close();
}

void FileIOWrappers::transferFileInParts(
    const std::string& srcPath,
    const std::string& destPath) {
  auto fileWriter = std::make_unique<fbpcf::io::FileWriter>(destPath);
  auto bufferedWriter = std::make_unique<fbpcf::io::BufferedWriter>(
      std::move(fileWriter), kBufferedWriterChunkSize);

  auto reader = std::make_unique<fbpcf::io::FileReader>(srcPath);
  auto bufferedReader = std::make_unique<fbpcf::io::BufferedReader>(
      std::move(reader), kBufferedReaderChunkSize);

  std::string newLine = "\n";
  while (!bufferedReader->eof()) {
    auto line = bufferedReader->readLine();
    bufferedWriter->writeString(line);
    bufferedWriter->writeString(newLine);
  }

  bufferedReader->close();
  bufferedWriter->close();
}

} // namespace fbpcf::io
