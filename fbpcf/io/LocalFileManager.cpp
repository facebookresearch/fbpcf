/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LocalFileManager.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "folly/Format.h"

#include "LocalInputStream.h"
#include "fbpcf/exception/PcfException.h"

namespace fbpcf {
std::unique_ptr<IInputStream> LocalFileManager::getInputStream(
    const std::string& fileName) {
  std::ifstream is{fileName, std::ios_base::binary};

  if (is.fail()) {
    throw PcfException{folly::sformat("Failed to open file {}", fileName)};
  }

  return std::make_unique<LocalInputStream>(std::move(is));
}

std::string LocalFileManager::read(const std::string& fileName) {
  auto stream = getInputStream(fileName);
  std::stringstream ss;
  ss << stream->get().rdbuf();

  return ss.str();
}

void LocalFileManager::write(
    const std::string& fileName,
    const std::string& data) {
  std::ofstream os{fileName};
  if (!os.is_open()) {
    throw PcfException{folly::sformat("Failed to open file {}", fileName)};
  }

  os << data;
}

std::string LocalFileManager::readBytes(
    const std::string& fileName,
    std::size_t start,
    std::size_t end) {
  if (start > end) {
    throw PcfException{folly::sformat(
        "Start byte: <{}> is larger than the end: <{}>", start, end)};
  }
  auto stream = getInputStream(fileName);
  std::istream& is = stream->get();

  // get length of file
  is.seekg(0, std::ios::end);
  size_t length = is.tellg();
  if (start > length) {
    throw PcfException{folly::sformat(
        "Start byte: <{}> is larger than the length: <{}>", start, length)};
  }
  is.seekg(start, std::ios::beg);

  auto validatedEnd = std::min(length, end);
  size_t bufferLength = validatedEnd - start;

  char buffer[bufferLength + 1];

  // read data as a block:
  if (is.readsome(buffer, bufferLength) != bufferLength) {
    throw PcfException{
        folly::sformat(
            "Hit exception when trying to read bytes from {}", fileName),
    };
  }

  buffer[bufferLength] = '\0';

  std::string s{buffer};
  return s;
}
} // namespace fbpcf
