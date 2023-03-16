/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <sstream>
#include <unordered_map>
#include "fbpcf/io/api/FileIOWrappers.h"

namespace fbpcf::mpc_std_lib::oram {
class OramMappingConfig {
 public:
  OramMappingConfig(
      std::unordered_map<std::string, uint32_t> breakdownMapping,
      bool wereBreakdownsFiltered,
      uint32_t filterIndex)
      : breakdownMapping_(breakdownMapping),
        wereBreakdownsFiltered_{wereBreakdownsFiltered},
        filterIndex_{filterIndex} {}

  std::string serializeAsString() {
    std::ostringstream s;
    boost::archive::text_oarchive oa(s);

    oa << breakdownMapping_ << wereBreakdownsFiltered_ << filterIndex_;
    return s.str();
  }

  void writeMappingToFile(std::string fileName) {
    fbpcf::io::FileIOWrappers::writeFile(fileName, serializeAsString());
  }

  static std::unique_ptr<OramMappingConfig> fromSerializedString(
      std::string str) {
    std::unordered_map<std::string, uint32_t> breakdownMapping{};
    bool wereBreakdownsFiltered;
    uint32_t filterIndex;

    std::istringstream s(str);

    boost::archive::text_iarchive ia(s);
    ia >> breakdownMapping;
    ia >> wereBreakdownsFiltered;
    ia >> filterIndex;

    return std::make_unique<OramMappingConfig>(
        breakdownMapping, wereBreakdownsFiltered, filterIndex);
  }

  static std::unique_ptr<OramMappingConfig> readMappingFromFile(
      std::string fileName) {
    return fromSerializedString(fbpcf::io::FileIOWrappers::readFile(fileName));
  }

  const std::unordered_map<std::string, uint32_t>& getBreakdownMapping() const {
    return breakdownMapping_;
  }

  bool wereBreakdownsFiltered() const {
    return wereBreakdownsFiltered_;
  }

  uint32_t getFilterIndex() {
    return filterIndex_;
  }

 private:
  std::unordered_map<std::string, uint32_t> breakdownMapping_{};
  bool wereBreakdownsFiltered_ = false;
  uint32_t filterIndex_ = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
