/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "folly/Random.h"

#include "../../InputData.h"
#include "GenFakeData.h"

namespace private_lift {
double GenFakeData::genAdjustedPurchaseRate(
    bool isTest,
    double purchaseRate,
    double incrementalityRate) {
  double adjstedPurchaseRate = purchaseRate;
  if (isTest) {
    adjstedPurchaseRate = purchaseRate + (incrementalityRate / 2.0);
    if (adjstedPurchaseRate > 1.0) {
      throw std::invalid_argument(
          ">1.0 incrementality_rate + purchase_rate is not yet supported");
    }
  } else {
    adjstedPurchaseRate = purchaseRate - (incrementalityRate / 2.0);
    if (adjstedPurchaseRate < 0.0) {
      throw std::invalid_argument(
          "Incrementality rate cannot be significantly higher than the purchase rate");
    }
  }
  return adjstedPurchaseRate;
}

GenFakeData::LiftInputColumns GenFakeData::genOneFakeLine(
    const std::string& id,
    double opportunityRate,
    double testRate,
    double purchaseRate,
    double incrementalityRate,
    int32_t epoch,
    int32_t numConversions) {
  LiftInputColumns oneLine;
  oneLine.id = id;
  oneLine.opportunity = folly::Random::secureRandDouble01() < opportunityRate;
  oneLine.test_flag =
      oneLine.opportunity && folly::Random::secureRandDouble01() < testRate;
  purchaseRate = genAdjustedPurchaseRate(
      oneLine.test_flag, purchaseRate, incrementalityRate);
  bool hasPurchase = folly::Random::secureRandDouble01() < purchaseRate;
  oneLine.opportunity_timestamp =
      oneLine.opportunity ? folly::Random::secureRand32(100) + epoch : 0;

  if (!hasPurchase) {
    oneLine.event_timestamps.resize(numConversions, 0);
    oneLine.values.resize(numConversions, 0);
  } else {
    uint32_t randomCount = numConversions - folly::Random::secureRand32(numConversions);
    std::vector<std::pair<int32_t, int32_t>> tsValVec;
    for (int32_t i = 0; i < numConversions; i++) {
      if (randomCount > 0) {
        int32_t timeStamp = folly::Random::secureRand32(100) + epoch;
        int32_t value = folly::Random::secureRand32(100) + 1;
        tsValVec.push_back(std::make_pair(timeStamp, value));
        randomCount--;
      } else {
        tsValVec.push_back(std::make_pair(0, 0));
      }
    }

    // sort by timestamp
    std::sort(tsValVec.begin(), tsValVec.end(), [](auto &a, auto &b) {
      return a.first < b.first;
    });

    for (auto [ts, value] : tsValVec) {
      oneLine.event_timestamps.push_back(ts);
      oneLine.values.push_back(value);
    }
  }

  return oneLine;
}

void GenFakeData::genFakePublisherInputFile(
    std::string filename,
    int32_t numRows,
    double opportunityRate,
    double testRate,
    double purchaseRate,
    double incrementalityRate,
    int32_t epoch) {
  std::ofstream publisherFile{filename};

  // publisher header: id_,opportunity,test_flag,opportunity_timestamp
  publisherFile << "id_,opportunity,test_flag,opportunity_timestamp\n";

  for (auto i = 0; i < numRows; i++) {
    // generate one row of fake data
    LiftInputColumns oneLine = genOneFakeLine(
        std::to_string(i),
        opportunityRate,
        testRate,
        purchaseRate,
        incrementalityRate,
        epoch,
        1);

    // write one row to publisher fake data file
    std::string publisherRow = oneLine.id + "," +
        (oneLine.opportunity ? "1," : "0,") +
        (oneLine.test_flag ? "1," : "0,") +
        std::to_string(oneLine.opportunity_timestamp);
    publisherFile << publisherRow << '\n';
  }
}

void GenFakeData::genFakePartnerInputFile(
    std::string filename,
    int32_t numRows,
    double opportunityRate,
    double testRate,
    double purchaseRate,
    double incrementalityRate,
    int32_t epoch,
    int32_t numConversions,
    bool omitValuesColumn) {
  std::ofstream partnerFile{filename};

  // partner header: id_,event_timestamps,values
  if (!omitValuesColumn) {
    partnerFile << "id_,event_timestamps,values\n";
  } else {
    partnerFile << "id_,event_timestamps\n";
  }

  for (auto i = 0; i < numRows; i++) {
    // generate one row of fake data
    LiftInputColumns oneLine = genOneFakeLine(
        std::to_string(i),
        opportunityRate,
        testRate,
        purchaseRate,
        incrementalityRate,
        epoch,
        numConversions);

    // write one row to publisher fake data file
    std::string eventTSString = "[";
    std::string valuesString = "[";
    for (auto j = 0; j < numConversions; j++) {
      eventTSString += std::to_string(oneLine.event_timestamps[j]);
      valuesString += std::to_string(oneLine.values[j]);
      if (j < numConversions - 1) {
        eventTSString += ",";
        valuesString += ",";
      } else {
        eventTSString += "]";
        valuesString += "]";
      }
    }
    if (!omitValuesColumn) {
      partnerFile << oneLine.id << "," << eventTSString << "," << valuesString
                << "\n";
    } else {
      // Again, skip "values" column if this is a valueless objective
      partnerFile << oneLine.id << "," << eventTSString << "\n";
    }
  }
}
} // namespace private_lift
