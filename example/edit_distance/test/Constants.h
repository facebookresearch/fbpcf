/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
namespace fbcpf::edit_distance {

const std::vector<std::string> kExpectedWords = {"temporary", "mug"};
const std::vector<std::string> kExpectedSenderMessages = {"box", "soft"};
const std::vector<std::string> kExpectedGuesses = {"curvy", "grain"};
const std::vector<std::string> kEmptyVector = {"", ""};

const int kExpectedThreshold = 100;
const std::vector<int64_t> kExpectedThresholdBatch = {
    kExpectedThreshold,
    kExpectedThreshold};

const int kExpectedDeleteCost = 35;
const std::vector<int64_t> kExpectedDeleteBatch = {
    kExpectedDeleteCost,
    kExpectedDeleteCost};

const int kExpectedInsertCost = 30;
const std::vector<int64_t> kExpectedInsertBatch = {
    kExpectedInsertCost,
    kExpectedInsertCost};

} // namespace fbcpf::edit_distance
