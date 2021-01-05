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

#include "../exception/PcfException.h"
#include "LocalInputStream.h"

namespace pcf {
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
  os << data;
}
} // namespace pcf
