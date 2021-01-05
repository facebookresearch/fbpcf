/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include "../../InputData.h"

namespace private_lift {
class GenFakeData {
 public:
  void genFakePublisherInputFile(
      std::string filename,
      int32_t numRows,
      double opportunityRate,
      double testRate,
      double purchaseRate,
      double incrementalityRate,
      int32_t epoch);
  void genFakePartnerInputFile(
      std::string filename,
      int32_t numRows,
      double opportunityRate,
      double testRate,
      double purchaseRate,
      double incrementalityRate,
      int32_t epoch,
      int32_t numConversions);

 private:
  struct LiftInputColumns {
    // publisher header: id_,opportunity,test_flag,opportunity_timestamp
    // partner header: id_,event_timestamps,values
    std::string id;
    bool opportunity;
    bool test_flag;
    int32_t opportunity_timestamp;
    std::vector<int32_t> event_timestamps;
    std::vector<int32_t> values;
  };
  LiftInputColumns genOneFakeLine(
      const std::string& id,
      double opportunityRate,
      double testRate,
      double purchaseRate,
      double incrementalityRate,
      int32_t epoch,
      int32_t numConversions);
  double genAdjustedPurchaseRate(
      bool isTest,
      double purchaseRate,
      double incrementalityRate);
};

} // namespace private_lift
