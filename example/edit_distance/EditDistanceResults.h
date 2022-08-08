/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "./MPCTypes.h" // @manual
#include "fbpcf/exception/exceptions.h"
#include "folly/dynamic.h"

namespace fbpcf::edit_distance {
struct EditDistanceResults {
  EditDistanceResults() = default;
  explicit EditDistanceResults(std::vector<folly::dynamic>& individualShares) {
    if (individualShares.empty()) {
      throw common::exceptions::ConstructionError("No shares to recover.");
    }
    size_t numRows = individualShares[0]["editDistanceShares"].size();

    for (auto& share : individualShares) {
      if (share["editDistanceShares"].size() != numRows ||
          share["receiverMessageShares"].size() != numRows) {
        throw common::exceptions::ConstructionError(
            "Recovered shares have inconsistent array size");
      }
    }

    editDistances = std::vector<int64_t>();
    receiverMessages = std::vector<std::string>();
    editDistances.reserve(numRows);
    receiverMessages.reserve(numRows);

    for (int i = 0; i < numRows; i++) {
      int64_t distance = 0;
      for (int j = 0; j < individualShares.size(); j++) {
        distance =
            distance ^ individualShares[j]["editDistanceShares"][i].asInt();
      }
      editDistances.push_back(distance);
    }

    for (int iRow = 0; iRow < numRows; iRow++) {
      std::string recoveredMessage = "";
      for (int iMessage = 0; iMessage < maxStringLength; iMessage++) {
        char c = 0;
        for (int iShare = 0; iShare < individualShares.size(); iShare++) {
          std::string strShare =
              individualShares[iShare]["receiverMessageShares"][iRow]
                  .asString();
          c ^= (strShare.size() > iMessage ? strShare[iMessage] : 0);
        }

        if (c == 0) {
          break;
        } else {
          recoveredMessage.push_back(c);
        }
      }
      receiverMessages.push_back(recoveredMessage);
    }
  }

  std::vector<int64_t> editDistances;
  std::vector<std::string> receiverMessages;
};
} // namespace fbpcf::edit_distance
