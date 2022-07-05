/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <future>
#include <memory>
#include <random>
#include <unordered_map>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/NonShufflerFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::shuffler {

template <int schedulerId>
std::vector<util::AttributionData> task(
    std::unique_ptr<IShuffler<util::SecretAttributionData<schedulerId>>>
        shuffler,
    const std::vector<util::AttributionData>& data) {
  auto secData = util::MpcAdapters<util::AttributionData, schedulerId>::
      processSecretInputs(data, 0);
  auto shuffled = shuffler->shuffle(secData, data.size());
  auto rst = util::MpcAdapters<util::AttributionData, schedulerId>::openToParty(
      shuffled, 0);
  return rst;
}

void shufflerTest(
    IShufflerFactory<util::SecretAttributionData<0>>& shufflerFactory0,
    IShufflerFactory<util::SecretAttributionData<1>>& shufflerFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto shuffler0 = shufflerFactory0.create();
  auto shuffler1 = shufflerFactory1.create();

  /* Generate test data for AttributionData type. */
  size_t size = 10;
  std::vector<uint64_t> adId(size);
  std::iota(adId.begin(), adId.end(), 0); // assign unique ids starting from 0.
  std::vector<uint64_t> conversionValue(size);
  std::iota(conversionValue.begin(), conversionValue.end(), 100);
  auto label = util::generateRandomBinary(size);

  std::vector<util::AttributionData> data(size);
  for (size_t i = 0; i < size; i++) {
    data[i] = util::AttributionData(
        util::AttributionValue(adId.at(i), conversionValue.at(i)), label.at(i));
  }

  auto future0 = std::async(task<0>, std::move(shuffler0), data);
  auto future1 = std::async(task<1>, std::move(shuffler1), data);
  auto rst = future0.get();
  future1.get();

  ASSERT_EQ(rst.size(), size);
  for (size_t i = 0; i < size; i++) {
    /* check consistensiy of each record */
    auto index = rst.at(i).value.adId; // ad id corresponds to the index
                                       // position of original data.
    ASSERT_EQ(rst.at(i).value.adId, data.at(index).value.adId);
    ASSERT_EQ(
        rst.at(i).value.conversionValue, data.at(index).value.conversionValue);
    ASSERT_EQ(rst.at(i).label, data.at(index).label);
  }
}

TEST(shufflerTestAttributionValue, testNonShuffler) {
  insecure::NonShufflerFactory<util::SecretAttributionData<0>> factory0;
  insecure::NonShufflerFactory<util::SecretAttributionData<1>> factory1;

  shufflerTest(factory0, factory1);
}

TEST(
    shufflerTestAttributionValue,
    testPermuteBasedShufflerWithAsWaksmanPermuter) {
  PermuteBasedShufflerFactory<util::SecretAttributionData<0>> factory0(
      0,
      1,
      std::make_unique<
          permuter::AsWaksmanPermuterFactory<util::AttributionData, 0>>(0, 1),
      std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<util::SecretAttributionData<1>> factory1(
      1,
      0,
      std::make_unique<
          permuter::AsWaksmanPermuterFactory<util::AttributionData, 1>>(1, 0),
      std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::shuffler
