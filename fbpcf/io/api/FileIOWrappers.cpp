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
#include "fbpcf/io/api/IOUtils.h"

namespace fbpcf::io {

std::string FileIOWrappers::readFile(const std::string& srcPath) {
  auto reader = std::make_unique<fbpcf::io::FileReader>(srcPath);
  auto bufferedReader =
      std::make_unique<fbpcf::io::BufferedReader>(std::move(reader));
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
  auto bufferedWriter =
      std::make_unique<fbpcf::io::BufferedWriter>(std::move(fileWriter));
  bufferedWriter->writeString(content);
  bufferedWriter->close();
}

void FileIOWrappers::transferFileInParts(
    const std::string& srcPath,
    const std::string& destPath) {
  auto fileWriter = std::make_unique<fbpcf::io::FileWriter>(destPath);
  auto bufferedWriter =
      std::make_unique<fbpcf::io::BufferedWriter>(std::move(fileWriter));

  auto reader = std::make_unique<fbpcf::io::FileReader>(srcPath);
  auto bufferedReader =
      std::make_unique<fbpcf::io::BufferedReader>(std::move(reader));

  std::string newLine = "\n";
  while (!bufferedReader->eof()) {
    auto line = bufferedReader->readLine();
    bufferedWriter->writeString(line);
    bufferedWriter->writeString(newLine);
  }

  bufferedReader->close();
  bufferedWriter->close();
}

bool FileIOWrappers::readCsv(
    const std::string& fileName,
    std::function<
        void(const std::vector<std::string>&, const std::vector<std::string>&)>
        readLine,
    std::function<void(const std::vector<std::string>&)> processHeader) {
  auto inlineReader = std::make_unique<fbpcf::io::FileReader>(fileName);
  auto inlineBufferedReader =
      std::make_unique<fbpcf::io::BufferedReader>(std::move(inlineReader));

  std::string line = inlineBufferedReader->readLine();
  auto header = IOUtils::splitByComma(line);
  processHeader(header);

  while (!inlineBufferedReader->eof()) {
    line = inlineBufferedReader->readLine();
    auto parts = IOUtils::splitByComma(line);
    readLine(header, parts);
  }
  inlineBufferedReader->close();
  return true;
}

} // namespace fbpcf::io
