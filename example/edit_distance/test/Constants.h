/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
namespace fbpcf::edit_distance {

inline const std::vector<std::string> kExpectedWords = {
    "temporary",
    "mug",
    "black-and-white",
    ""};
inline const std::vector<std::string> kExpectedSenderMessages = {
    "box",
    "soft",
    "hello",
    "empty"};
inline const std::vector<std::string> kExpectedGuesses = {
    "curvy",
    "grain",
    "black-and-white",
    "not-empty"};
inline const std::vector<int64_t> kExpectedDistances = {131, 81, 0, 315};
inline const std::vector<std::string> kExpectedReceiverMessages = {
    "b",
    "soft",
    "hello",
    ""};
inline const std::vector<std::string> kEmptyVector = {"", "", "", ""};

const int kExpectedThreshold = 100;
inline const std::vector<int64_t> kExpectedThresholdBatch = {
    kExpectedThreshold,
    kExpectedThreshold,
    kExpectedThreshold,
    kExpectedThreshold};

const int kExpectedDeleteCost = 35;
inline const std::vector<int64_t> kExpectedDeleteBatch = {
    kExpectedDeleteCost,
    kExpectedDeleteCost,
    kExpectedDeleteCost,
    kExpectedDeleteCost};

const int kExpectedInsertCost = 30;
inline const std::vector<int64_t> kExpectedInsertBatch = {
    kExpectedInsertCost,
    kExpectedInsertCost,
    kExpectedInsertCost,
    kExpectedInsertCost};

} // namespace fbpcf::edit_distance
