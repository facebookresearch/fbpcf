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

bool FileIOWrappers::readCsv(
    const std::string& fileName,
    std::function<
        void(const std::vector<std::string>&, const std::vector<std::string>&)>
        readLine,
    std::function<void(const std::vector<std::string>&)> processHeader) {
  auto inlineReader = std::make_unique<fbpcf::io::FileReader>(fileName);
  auto inlineBufferedReader = std::make_unique<fbpcf::io::BufferedReader>(
      std::move(inlineReader), kBufferedReaderChunkSize);

  std::string line = inlineBufferedReader->readLine();
  auto header = splitByComma(line);
  processHeader(header);

  while (!inlineBufferedReader->eof()) {
    // Split on commas, but if it looks like we're reading an array
    // like `[1, 2, 3]`, take the whole array
    line = inlineBufferedReader->readLine();
    auto parts = splitByComma(line);
    readLine(header, parts);
  }
  inlineBufferedReader->close();
  return true;
}

const std::vector<std::string> FileIOWrappers::split(
    std::string& str,
    const std::string& delim) {
  // Preprocessing step: Remove spaces if any
  str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
  std::vector<std::string> tokens;
  re2::RE2 rgx{delim};
  re2::StringPiece input{str}; // Wrap a StringPiece around it

  std::string token;
  while (RE2::Consume(&input, rgx, &token)) {
    tokens.push_back(token);
  }
  return tokens;
}

const std::vector<std::string> FileIOWrappers::splitByComma(std::string& str) {
  // split internally uses RE2 which relies on
  // consuming patterns. The pattern here indicates
  // it will get all the non commas [^,]. The surrounding () makes it
  // a capture group. ,? means there may or may not be a comma
  return split(str, "([^,]+),?");
}

} // namespace fbpcf::io
