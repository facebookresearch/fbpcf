/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "folly/logging/xlog.h"

#include "../common/GroupedLiftMetrics.h"

namespace private_lift {

constexpr int PUBLISHER = emp::ALICE;
constexpr int PARTNER = emp::BOB;
constexpr int QUICK_BITS = 32;
constexpr int FULL_BITS = 64;

template <int MY_ROLE>
constexpr auto privatelyShareInt = secret_sharing::privatelyShareInt<MY_ROLE>;
template <int MY_ROLE>
constexpr auto privatelyShareIntsFromPublisher =
    secret_sharing::privatelyShareIntsFromAlice<MY_ROLE>;
template <int MY_ROLE>
constexpr auto privatelyShareIntsFromPartner =
    secret_sharing::privatelyShareIntsFromBob<MY_ROLE>;
template <int MY_ROLE>
constexpr auto privatelyShareBitsFromPublisher =
    secret_sharing::privatelyShareBitsFromAlice<MY_ROLE>;
template <int MY_ROLE>
constexpr auto privatelyShareBitsFromPartner =
    secret_sharing::privatelyShareBitsFromBob<MY_ROLE>;
template <int MY_ROLE>
constexpr auto privatelyShareIntArraysFromPartner =
    secret_sharing::privatelyShareIntArraysNoPaddingFromBob<MY_ROLE>;

template <int MY_ROLE>
template <class T>
T OutputMetrics<MY_ROLE>::reveal(const emp::Integer& empInteger) const {
  return shouldUseXorEncryption() ? empInteger.reveal<T>(emp::XOR)
                                  : empInteger.reveal<T>();
}

template <int MY_ROLE>
std::string OutputMetrics<MY_ROLE>::playGame() {
  validateNumRows();
  initNumGroups();
  initShouldSkipValues();
  initBitsForValues();
  calculateAll();

  // Print the outputs
  XLOG(INFO) << "\nEMP Output (Role=" << MY_ROLE << "):\n" << metrics_;

  // Print each subgroup header. Note that the publisher won't know anything
  // about the group header (only a generic index for which group we are
  // currently outputting.
  for (auto i = 0; i < subgroupMetrics_.size(); ++i) {
    XLOG(INFO) << "\nSubgroup [" << i << "] results:";
    if (MY_ROLE == PARTNER) {
      auto features = inputData_.getGroupIdToFeatures().at(i);
      std::stringstream headerSs;
      for (auto j = 0; j < features.size(); ++j) {
        auto featureHeader = inputData_.getFeatureHeader().at(j);
        headerSs << featureHeader << "=" << features.at(j);
        if (j + 1 < features.size()) {
          headerSs << ", ";
        }
      }
      XLOG(INFO) << headerSs.str();
    } else {
      XLOG(INFO) << "(Feature header unknown to publisher)";
    }

    auto subgroupMetrics = subgroupMetrics_[i];
    XLOG(INFO) << subgroupMetrics;
  }
  return toJson();
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::writeOutputToFile(std::ostream& outfile) {
  // Start by outputting the overall results
  outfile << "Overall"
          << ",";
  outfile << metrics_.testEvents << ",";
  outfile << metrics_.controlEvents << ",";
  // Value metrics are only relevant for conversion lift
  if (inputData_.getLiftGranularityType() ==
      InputData::LiftGranularityType::Conversion) {
    outfile << metrics_.testValue << ",";
    outfile << metrics_.controlValue << ",";
    outfile << metrics_.testSquared << ",";
    outfile << metrics_.controlSquared << ",";
  }
  outfile << metrics_.testPopulation << ",";
  outfile << metrics_.controlPopulation << "\n";

  // Then output results for each group
  // Print each subgroup header. Note that the publisher won't know anything
  // about the group header (only a generic index for which group we are
  // currently outputting.
  for (auto i = 0; i < subgroupMetrics_.size(); ++i) {
    auto subOut = subgroupMetrics_.at(i);
    if (MY_ROLE == PARTNER) {
      auto features = inputData_.getGroupIdToFeatures().at(i);
      for (auto j = 0; j < features.size(); ++j) {
        auto featureHeader = inputData_.getFeatureHeader().at(j);
        outfile << featureHeader << "=" << features.at(j);
        if (j + 1 < features.size()) {
          outfile << " AND ";
        }
      }
      outfile << ",";
    } else {
      outfile << "Subgroup " << i << ",";
    }

    outfile << subOut.testEvents << ",";
    outfile << subOut.controlEvents << ",";
    // Value metrics are only relevant for conversion lift
    if (inputData_.getLiftGranularityType() ==
        InputData::LiftGranularityType::Conversion) {
      outfile << subOut.testValue << ",";
      outfile << subOut.controlValue << ",";
      outfile << subOut.testSquared << ",";
      outfile << subOut.controlSquared << ",";
    }
    outfile << subOut.testPopulation << ",";
    outfile << subOut.controlPopulation << "\n";
  }
}

template <int MY_ROLE>
std::string OutputMetrics<MY_ROLE>::toJson() const {
  GroupedLiftMetrics groupedLiftMetrics;
  groupedLiftMetrics.metrics = metrics_.toLiftMetrics();
  std::transform(
      subgroupMetrics_.begin(),
      subgroupMetrics_.end(),
      std::back_inserter(groupedLiftMetrics.subGroupMetrics),
      [](auto const& p) { return p.second.toLiftMetrics(); });
  return groupedLiftMetrics.toJson();
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::validateNumRows() {
  auto numRows = privatelyShareInt<MY_ROLE>(n_);
  auto publisherNumRows = numRows.publisherInt().template reveal<int64_t>();
  auto partnerNumRows = numRows.partnerInt().template reveal<int64_t>();

  if (publisherNumRows != partnerNumRows) {
    // Using LOG(FATAL) will make the publisher hang since they'll never get the
    // reveal for some reason.
    XLOG(ERR) << "The publisher has " << publisherNumRows
              << " rows in their input, while the partner has "
              << partnerNumRows << " rows.";
    exit(1);
  }
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::initNumGroups() {
  XLOG(INFO) << "Set up number of groups and groupId share";
  // emp::Integer operates on int64_t values, so we do a static cast here
  // This is fine since we shouldn't be handling more than 2^63-1 groups...
  auto numGroups = static_cast<int64_t>(inputData_.getNumGroups());
  emp::Integer numGroupsInteger{INT_SIZE, numGroups, PARTNER};
  numGroups_ = numGroupsInteger.reveal<int64_t>();
  // We pre-share the bitmasks for each group since they will be used
  // multiple times throughout the computation
  for (auto i = 0; i < numGroups_; ++i) {
    groupBitmasks_[i] =
        privatelyShareBitsFromPartner<MY_ROLE>(inputData_.bitmaskFor(i), n_);
  }
  XLOG(INFO) << "Will be computing metrics for " << numGroups_ << " subgroups";
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::initShouldSkipValues() {
  XLOG(INFO) << "Determine if value-based calculations should be skipped";
  bool hasValues = inputData_.getPurchaseValueArrays().empty();
  emp::Bit hasValuesBit{hasValues, PARTNER};
  shouldSkipValues_ = hasValuesBit.reveal<bool>();
  XLOG(INFO) << "shouldSkipValues = " << shouldSkipValues_;
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::initBitsForValues() {
  if (!shouldSkipValues_) {
    XLOG(INFO) << "Set up number of bits needed for purchase value sharing";
    auto valueBits = static_cast<int64_t>(inputData_.getNumBitsForValue());
    auto valueSquaredBits =
        static_cast<int64_t>(inputData_.getNumBitsForValueSquared());
    emp::Integer valueBitsInteger{INT_SIZE, valueBits, PARTNER};
    emp::Integer valueSquaredBitsInteger{INT_SIZE, valueSquaredBits, PARTNER};
    // TODO: Figure out why this isn't working when using values other than
    // 32/64
    valueBits_ = valueBitsInteger.reveal<int64_t>() <= QUICK_BITS ? QUICK_BITS
                                                                  : FULL_BITS;
    valueSquaredBits_ = valueSquaredBitsInteger.reveal<int64_t>() <= QUICK_BITS
        ? QUICK_BITS
        : FULL_BITS;
    XLOG(INFO) << "Num bits for values: " << valueBits_;
    XLOG(INFO) << "Num bits for values squared: " << valueSquaredBits_;
  }
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::calculateAll() {
  XLOG(INFO) << "Start calculation of output metrics";

  std::vector<std::vector<emp::Integer>> purchaseValueArrays;
  if (!shouldSkipValues_) {
    XLOG(INFO) << "Share purchase values";
    purchaseValueArrays = privatelyShareIntArraysFromPartner<MY_ROLE>(
        inputData_.getPurchaseValueArrays(),
        n_, /* numVals */
        numConversionsPerUser_ /* arraySize */,
        valueBits_ /* bitLen */);
  }

  auto validPurchaseArrays = calculateValidPurchases();

  std::vector<std::vector<emp::Integer>> purchaseValueSquaredArrays;

  // If this is (value-based) conversion lift, we also need to share purchase
  // values squared
  if (!shouldSkipValues_ &&
      inputData_.getLiftGranularityType() ==
          InputData::LiftGranularityType::Conversion) {
    purchaseValueSquaredArrays = privatelyShareIntArraysFromPartner<MY_ROLE>(
        inputData_.getPurchaseValueSquaredArrays(),
        n_, /* numVals */
        numConversionsPerUser_ /* arraySize */,
        valueSquaredBits_ /* bitLen */);
  }

  calculateStatistics(
      GroupType::TEST,
      purchaseValueArrays,
      purchaseValueSquaredArrays,
      validPurchaseArrays);
  calculateStatistics(
      GroupType::CONTROL,
      purchaseValueArrays,
      purchaseValueSquaredArrays,
      validPurchaseArrays);
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::calculateStatistics(
    const OutputMetrics::GroupType& groupType,
    const std::vector<std::vector<emp::Integer>>& purchaseValueArrays,
    const std::vector<std::vector<emp::Integer>>& purchaseValueSquaredArrays,
    const std::vector<std::vector<emp::Bit>>& validPurchaseArrays) {
  XLOG(INFO) << "Calculate " << getGroupTypeStr(groupType)
             << " events, value, and value squared";
  auto bits = calculatePopulation(groupType, inputData_.getControlPopulation());
  if (groupType == GroupType::TEST) {
    bits = calculatePopulation(groupType, inputData_.getTestPopulation());
  }
  auto eventArrays = calculateEvents(groupType, bits, validPurchaseArrays);

  // If this is (value-based) conversion lift, calculate value metrics now
  if (!shouldSkipValues_ &&
      inputData_.getLiftGranularityType() ==
          InputData::LiftGranularityType::Conversion) {
    calculateValue(groupType, purchaseValueArrays, eventArrays);
    calculateValueSquared(groupType, purchaseValueSquaredArrays, eventArrays);
  }
}

template <int MY_ROLE>
std::vector<emp::Bit> OutputMetrics<MY_ROLE>::calculatePopulation(
    const OutputMetrics::GroupType& groupType,
    const std::vector<int64_t> populationVec) {
  XLOG(INFO) << "Calculate " << getGroupTypeStr(groupType) << " population";
  const std::vector<emp::Bit> populationBits =
      privatelyShareBitsFromPublisher<MY_ROLE>(populationVec, n_);
  // Since testSum/controlSum is only dependent on publisher data, we compute on
  // the publisher side then just share the value over to the partner. Note
  // however that we still need to share emp::Bit for the population to compute
  // the subgroup data since the publisher doesn't know group membership
  auto theSum = std::accumulate(populationVec.begin(), populationVec.end(), 0);
  emp::Integer sumInt{INT_SIZE, theSum, PUBLISHER};
  if (groupType == GroupType::TEST) {
    metrics_.testPopulation = reveal<int64_t>(sumInt);
  } else {
    metrics_.controlPopulation = reveal<int64_t>(sumInt);
  }
  // And compute for subgroups
  for (auto i = 0; i < numGroups_; ++i) {
    auto groupBits =
        secret_sharing::multiplyBitmask(populationBits, groupBitmasks_.at(i));
    if (groupType == GroupType::TEST) {
      subgroupMetrics_[i].testPopulation = sum(groupBits);
    } else {
      subgroupMetrics_[i].controlPopulation = sum(groupBits);
    }
  }
  return populationBits;
}

template <int MY_ROLE>
std::vector<std::vector<emp::Bit>>
OutputMetrics<MY_ROLE>::calculateValidPurchases() {
  // TODO: We're using 32 bits for timestamps along with an offset setting the
  // epoch to 2019-01-01. This will break in the year 2087.
  XLOG(INFO) << "Share opportunity timestamps";
  const std::vector<emp::Integer> opportunityTimestamps =
      privatelyShareIntsFromPublisher<MY_ROLE>(
          inputData_.getOpportunityTimestamps(), n_, QUICK_BITS);
  XLOG(INFO) << "Share purchase timestamps";
  const std::vector<std::vector<emp::Integer>> purchaseTimestampArrays =
      privatelyShareIntArraysFromPartner<MY_ROLE>(
          inputData_.getPurchaseTimestampArrays(),
          n_, /* numVals */
          numConversionsPerUser_ /* arraySize */,
          QUICK_BITS /* bitLen */);

  XLOG(INFO) << "Calculate valid purchases";
  return secret_sharing::zip_and_map<
      emp::Integer,
      std::vector<emp::Integer>,
      std::vector<emp::Bit>>(
      opportunityTimestamps,
      purchaseTimestampArrays,
      [](emp::Integer oppTs,
         std::vector<emp::Integer> purchaseTsArray) -> std::vector<emp::Bit> {
        std::vector<emp::Bit> vec;
        for (const auto& purchaseTs : purchaseTsArray) {
          const emp::Integer ten{purchaseTs.size(), 10, emp::PUBLIC};
          vec.push_back(purchaseTs + ten > oppTs);
        }
        return vec;
      });
}

template <int MY_ROLE>
std::vector<std::vector<emp::Bit>> OutputMetrics<MY_ROLE>::calculateEvents(
    const OutputMetrics::GroupType& groupType,
    const std::vector<emp::Bit>& populationBits,
    const std::vector<std::vector<emp::Bit>>& validPurchaseArrays) {
  XLOG(INFO) << "Calculate " << getGroupTypeStr(groupType) << " events";
  std::vector<std::vector<emp::Bit>> eventArrays = secret_sharing::
      zip_and_map<emp::Bit, std::vector<emp::Bit>, std::vector<emp::Bit>>(
          populationBits,
          validPurchaseArrays,
          [](emp::Bit isUser, std::vector<emp::Bit> validPurchaseArray)
              -> std::vector<emp::Bit> {
            std::vector<emp::Bit> vec;
            for (const auto& validPurchase : validPurchaseArray) {
              vec.push_back(isUser & validPurchase);
            }
            return vec;
          });

  if (groupType == GroupType::TEST) {
    metrics_.testEvents = sum(eventArrays);
  } else {
    metrics_.controlEvents = sum(eventArrays);
  }

  // And compute for subgroups
  for (auto i = 0; i < numGroups_; ++i) {
    auto groupBits =
        secret_sharing::multiplyBitmask(eventArrays, groupBitmasks_.at(i));
    groupType == GroupType::TEST
        ? subgroupMetrics_[i].testEvents = sum(groupBits)
        : subgroupMetrics_[i].controlEvents = sum(groupBits);
  }
  return eventArrays;
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::calculateValue(
    const OutputMetrics::GroupType& groupType,
    const std::vector<std::vector<emp::Integer>>& purchaseValueArrays,
    const std::vector<std::vector<emp::Bit>>& eventArrays) {
  XLOG(INFO) << "Calculate " << getGroupTypeStr(groupType) << " value";
  std::vector<std::vector<emp::Integer>> valueArrays =
      secret_sharing::zip_and_map<
          std::vector<emp::Bit>,
          std::vector<emp::Integer>,
          std::vector<emp::Integer>>(
          eventArrays,
          purchaseValueArrays,
          [](std::vector<emp::Bit> testEvents,
             std::vector<emp::Integer> purchaseValues)
              -> std::vector<emp::Integer> {
            std::vector<emp::Integer> vec;
            if (testEvents.size() != purchaseValues.size()) {
              XLOG(FATAL)
                  << "Numbers of test event bits and/or purchase values are inconsistent.";
            }
            for (auto i = 0; i < testEvents.size(); ++i) {
              const emp::Integer zero =
                  emp::Integer{purchaseValues.at(i).size(), 0, emp::PUBLIC};
              vec.emplace_back(
                  emp::If(testEvents.at(i), purchaseValues.at(i), zero));
            }
            return vec;
          });

  if (groupType == GroupType::TEST) {
    metrics_.testValue = sum(valueArrays);
  } else {
    metrics_.controlValue = sum(valueArrays);
  }

  // And compute for subgroups
  for (auto i = 0; i < numGroups_; ++i) {
    auto groupInts =
        secret_sharing::multiplyBitmask(valueArrays, groupBitmasks_.at(i));
    if (groupType == GroupType::TEST) {
      subgroupMetrics_[i].testValue = sum(groupInts);
    } else {
      subgroupMetrics_[i].controlValue = sum(groupInts);
    }
  }
}

template <int MY_ROLE>
void OutputMetrics<MY_ROLE>::calculateValueSquared(
    const OutputMetrics::GroupType& groupType,
    const std::vector<std::vector<emp::Integer>>& purchaseValueSquaredArrays,
    const std::vector<std::vector<emp::Bit>>& eventArrays) {
  XLOG(INFO) << "Calculate " << getGroupTypeStr(groupType) << " value squared";
  std::vector<emp::Integer> squaredValues = secret_sharing::zip_and_map<
      std::vector<emp::Bit>,
      std::vector<emp::Integer>,
      emp::Integer>(
      eventArrays,
      purchaseValueSquaredArrays,
      [](std::vector<emp::Bit> events,
         std::vector<emp::Integer> purchaseValuesSquared) -> emp::Integer {
        emp::Integer sumSquared{
            purchaseValuesSquared.at(0).size(), 0, emp::PUBLIC};
        if (events.size() != purchaseValuesSquared.size()) {
          XLOG(FATAL)
              << "Numbers of event bits and purchase values squared are inconsistent.";
        }
        emp::Bit tookAccumulationAlready{false, emp::PUBLIC};
        for (auto i = 0; i < events.size(); ++i) {
          // If this event is valid and we haven't taken the accumulation yet,
          // use this value as the sumSquared accumulation.
          // emp::If(condition, true_case, false_case)
          sumSquared = emp::If(
              events.at(i) & !tookAccumulationAlready,
              purchaseValuesSquared.at(i),
              sumSquared);
          // Always make sure we keep tookAccumulationAlready up-to-date
          tookAccumulationAlready = tookAccumulationAlready | events.at(i);
        }
        return sumSquared;
      });

  if (groupType == GroupType::TEST) {
    metrics_.testSquared = sum(squaredValues);
  } else {
    metrics_.controlSquared = sum(squaredValues);
  }

  // And compute for subgroups
  for (auto i = 0; i < numGroups_; ++i) {
    auto groupInts = secret_sharing::multiplyBitmask(
        squaredValues, groupBitmasks_.at(i));
    if (groupType == GroupType::TEST) {
      subgroupMetrics_[i].testSquared = sum(groupInts);
    } else {
      subgroupMetrics_[i].controlSquared = sum(groupInts);
    }
  }
}

template <int MY_ROLE>
int64_t OutputMetrics<MY_ROLE>::sum(const std::vector<emp::Integer>& in) const {
  return shouldUseXorEncryption() ? emp_utils::sum<emp::XOR>(in)
                                  : emp_utils::sum<emp::PUBLIC>(in);
}

template <int MY_ROLE>
int64_t OutputMetrics<MY_ROLE>::sum(const std::vector<emp::Bit>& in) const {
  return sum(emp_utils::bitsToInts(in));
}

template <int MY_ROLE>
int64_t OutputMetrics<MY_ROLE>::sum(
    const std::vector<std::vector<emp::Bit>>& in) const {
  // flatten the 2D vector into 1D
  // TODO: this can be optimizing by specializing this use case so we don't have
  // to make a copy of the data
  std::vector<emp::Bit> accum;
  for (auto& sub : in) {
    accum.insert(std::end(accum), std::begin(sub), std::end(sub));
  }
  return sum(accum);
}

template <int MY_ROLE>
int64_t OutputMetrics<MY_ROLE>::sum(
    const std::vector<std::vector<emp::Integer>>& in) const {
  // flatten the 2D vector into 1D
  // TODO: this can be optimizing by specializing this use case so we don't have
  // to make a copy of the data
  std::vector<emp::Integer> accum;
  for (auto& sub : in) {
    accum.insert(std::end(accum), std::begin(sub), std::end(sub));
  }
  return sum(accum);
}

} // namespace private_lift
