/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <fbpcf/scheduler/gate_keeper/IGate.h>
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/GateKeeper.h"
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::scheduler {

const bool unsafe = false;

void testLevel(
    std::vector<std::unique_ptr<IGate>> level,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> expectedNormalWires,
    std::vector<std::vector<IScheduler::WireId<IScheduler::Boolean>>>
        expectedCompositeWires) {
  ASSERT_EQ(
      level.size(), expectedNormalWires.size() + expectedCompositeWires.size());
  int normalWireIndex = 0;
  int compositeWireIndex = 0;
  for (auto i = 0; i < level.size(); ++i) {
    IGate* gate = level.at(i).get();
    INormalGate<IScheduler::Boolean>* normalGate =
        dynamic_cast<INormalGate<IScheduler::Boolean>*>(gate);
    ICompositeGate* compositeGate = dynamic_cast<ICompositeGate*>(gate);
    ASSERT_TRUE(normalGate == nullptr ^ compositeGate == nullptr);
    if (normalGate != nullptr) {
      EXPECT_EQ(
          normalGate->getWireId().getId(),
          expectedNormalWires.at(normalWireIndex).getId());
      normalWireIndex++;
    } else {
      auto outputWireIds = compositeGate->getOutputWireIds();
      auto expectedOutputWireIds =
          expectedCompositeWires.at(compositeWireIndex);
      ASSERT_EQ(outputWireIds.size(), expectedOutputWireIds.size());
      for (auto j = 0; j < outputWireIds.size(); ++j) {
        EXPECT_EQ(
            outputWireIds.at(j).getId(), expectedOutputWireIds.at(j).getId());
      }
      compositeWireIndex++;
    }
  }
}

TEST(GateKeeperTest, TestAddAndRemoveGates) {
  std::shared_ptr<IWireKeeper> wireKeeper =
      WireKeeper::createWithVectorArena<unsafe>();
  auto gateKeeper = std::make_unique<GateKeeper>(wireKeeper);

  // Non-batching API

  // Level 0
  auto wire1 = gateKeeper->inputGate(true);
  auto wire2 = gateKeeper->inputGate(false);

  // Level 1
  auto wire3 = gateKeeper->normalGate(
      INormalGate<IScheduler::Boolean>::GateType::NonFreeAnd, wire1, wire2);

  EXPECT_EQ(gateKeeper->getFirstUnexecutedLevel(), 0);

  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire1, wire2}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire3}, {});

  // Level 2
  auto wire4 = gateKeeper->normalGate(
      INormalGate<IScheduler::Boolean>::GateType::AsymmetricXOR, wire1, wire2);

  // Level 3
  auto wire5 = gateKeeper->outputGate(wire4, 0);
  auto wire6 = gateKeeper->outputGate(wire3, 1);

  // Level 5
  auto wire7 = gateKeeper->normalGate(
      INormalGate<IScheduler::Boolean>::GateType::NonFreeAnd, wire3, wire6);

  // Level 2
  auto wire8 = gateKeeper->inputGate(true);
  auto wire9 = gateKeeper->normalGate(
      INormalGate<IScheduler::Boolean>::GateType::AsymmetricNot, wire4);

  EXPECT_EQ(gateKeeper->getFirstUnexecutedLevel(), 2);

  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire4, wire8, wire9}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire5, wire6}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire7}, {});

  // Batching API

  // Level 6
  wire1 = gateKeeper->inputGateBatch({true, false});
  wire2 = gateKeeper->inputGateBatch({false, true});

  // Level 7
  wire3 = gateKeeper->normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType::NonFreeAnd, wire1, wire2);

  EXPECT_EQ(gateKeeper->getFirstUnexecutedLevel(), 6);

  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire1, wire2}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire3}, {});

  // Level 8
  wire4 = gateKeeper->normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType::AsymmetricXOR, wire1, wire2);

  // Level 9
  wire5 = gateKeeper->outputGateBatch(wire4, 0);
  wire6 = gateKeeper->outputGateBatch(wire3, 1);

  // Level 11
  wire7 = gateKeeper->normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType::NonFreeAnd, wire3, wire6);

  // Level 8
  wire8 = gateKeeper->inputGateBatch({true, true});
  wire9 = gateKeeper->normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType::AsymmetricNot, wire4);

  EXPECT_EQ(gateKeeper->getFirstUnexecutedLevel(), 8);

  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire4, wire8, wire9}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire5, wire6}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {}, {});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {wire7}, {});

  EXPECT_EQ(gateKeeper->getFirstUnexecutedLevel(), 12);
}

TEST(GateKeeperTest, TestCompositeGates) {
  std::shared_ptr<IWireKeeper> wireKeeper =
      WireKeeper::createWithVectorArena<unsafe>();
  auto gateKeeper = std::make_unique<GateKeeper>(wireKeeper);

  // level 0
  auto leftWire = gateKeeper->inputGate(true);
  std::vector<IScheduler::WireId<IScheduler::Boolean>> rightWires;
  for (int i = 0; i < 10; ++i) {
    rightWires.push_back(gateKeeper->inputGate(false));
  }

  auto wires1 = gateKeeper->compositeGate(
      ICompositeGate::GateType::FreeAnd, leftWire, rightWires);

  // level 1
  auto wires2 = gateKeeper->compositeGate(
      ICompositeGate::GateType::NonFreeAnd, leftWire, rightWires);

  std::vector<IScheduler::WireId<IScheduler::Boolean>> expectedWires0 = {
      leftWire};
  expectedWires0.insert(
      expectedWires0.end(), rightWires.begin(), rightWires.end());
  testLevel(gateKeeper->popFirstUnexecutedLevel(), expectedWires0, {wires1});
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {}, {wires2});

  // level 2
  auto leftWire2 = gateKeeper->normalGate(
      INormalGate<IScheduler::Boolean>::GateType::AsymmetricXOR,
      leftWire,
      wires2[0]);

  std::vector<IScheduler::WireId<IScheduler::Boolean>> rightWires2;
  auto lastLevelNotInput = leftWire;
  // levels 2 -> 129
  for (int i = 0; i < 64; ++i) {
    // level 2i + 2
    auto notGateWire = gateKeeper->normalGate(
        INormalGate<IScheduler::Boolean>::GateType::SymmetricNot,
        lastLevelNotInput);
    // level 2i + 3
    auto andGateWire = gateKeeper->normalGate(
        INormalGate<IScheduler::Boolean>::GateType::NonFreeAnd,
        leftWire2,
        notGateWire);

    lastLevelNotInput = andGateWire;
    rightWires2.push_back(notGateWire);
    rightWires2.push_back(andGateWire);
  }

  // level 130
  auto wires3 = gateKeeper->compositeGate(
      ICompositeGate::GateType::FreeAnd, leftWire2, rightWires2);
  EXPECT_EQ(wires3.size(), 128);

  // level 131
  auto wires4 = gateKeeper->compositeGate(
      ICompositeGate::GateType::NonFreeAnd, leftWire2, rightWires2);
  EXPECT_EQ(wires4.size(), 128);

  // check level 2
  expectedWires0 = {leftWire2, rightWires2[0]};
  testLevel(gateKeeper->popFirstUnexecutedLevel(), expectedWires0, {});

  // check levels 3 -> 129
  for (int i = 1; i < 128; ++i) {
    testLevel(gateKeeper->popFirstUnexecutedLevel(), {rightWires2[i]}, {});
  }

  // check level 130
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {}, {wires3});

  // check level 131
  testLevel(gateKeeper->popFirstUnexecutedLevel(), {}, {wires4});
}

} // namespace fbpcf::scheduler
