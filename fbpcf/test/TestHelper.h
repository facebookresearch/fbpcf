/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gtest/gtest.h>

#include <emmintrin.h>
#include <smmintrin.h>
#include <stdexcept>

#include <fbpcf/scheduler/IArithmeticScheduler.h>
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/NetworkPlaintextSchedulerFactory.h"
#include "fbpcf/scheduler/PlaintextSchedulerFactory.h"
#include "fbpcf/scheduler/SchedulerHelper.h"

namespace fbpcf {

template <typename T>
inline void testVectorEq(
    const std::vector<T>& left,
    const std::vector<T>& right) {
  ASSERT_EQ(left.size(), right.size());
  for (size_t i = 0; i < left.size(); i++) {
    EXPECT_EQ(left.at(i), right.at(i)) << "at position: " << i;
  }
}

template <typename T, size_t size>
inline void testArrayEq(
    const std::array<T, size>& left,
    const std::array<T, size>& right) {
  for (size_t i = 0; i < left.size(); i++) {
    EXPECT_EQ(left.at(i), right.at(i)) << "at position: " << i;
  }
}

template <typename T>
inline void testPairEq(
    const std::pair<T, T>& left,
    const std::pair<T, T>& right) {
  EXPECT_EQ(left.first, right.first);
  EXPECT_EQ(left.second, right.second);
}

inline bool compareM128i(__m128i left, __m128i right) {
  __m128i vcmp = _mm_xor_si128(left, right);
  return _mm_testz_si128(vcmp, vcmp);
}

inline void testEq(
    const std::vector<__m128i>& src0,
    const std::vector<__m128i>& src1) {
  ASSERT_EQ(src0.size(), src1.size());
  for (size_t i = 0; i < src0.size(); i++) {
    EXPECT_TRUE(compareM128i(src0.at(i), src1.at(i)));
  }
}

enum class SchedulerType { Plaintext, NetworkPlaintext, Eager, Lazy };

inline std::string getSchedulerName(SchedulerType schedulerType) {
  switch (schedulerType) {
    case SchedulerType::Plaintext:
      return "PlaintextScheduler";
    case SchedulerType::NetworkPlaintext:
      return "NetworkPlaintextScheduler";
    case SchedulerType::Eager:
      return "EagerScheduler";
    case SchedulerType::Lazy:
      return "LazyScheduler";
  }
}

using SchedulerCreator = std::function<std::unique_ptr<scheduler::IScheduler>(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory)>;

template <bool unsafe>
inline SchedulerCreator getSchedulerCreator(SchedulerType schedulerType) {
  switch (schedulerType) {
    case SchedulerType::Plaintext:
      return [](int myId,
                engine::communication::IPartyCommunicationAgentFactory&
                    communicationAgentFactory) {
        return scheduler::PlaintextSchedulerFactory<unsafe>().create();
      };
    case SchedulerType::NetworkPlaintext:
      return [](int myId,
                engine::communication::IPartyCommunicationAgentFactory&
                    communicationAgentFactory) {
        return scheduler::NetworkPlaintextSchedulerFactory<unsafe>(
                   myId, communicationAgentFactory)
            .create();
      };
    case SchedulerType::Eager:
      return scheduler::createEagerSchedulerWithInsecureEngine<unsafe>;
    case SchedulerType::Lazy:
      return scheduler::createLazySchedulerWithInsecureEngine<unsafe>;
  }
}

using ArithmeticSchedulerCreator =
    std::function<std::unique_ptr<scheduler::IArithmeticScheduler>(
        int myId,
        engine::communication::IPartyCommunicationAgentFactory&
            communicationAgentFactory)>;

template <bool unsafe>
inline ArithmeticSchedulerCreator getArithmeticSchedulerCreator(
    SchedulerType schedulerType) {
  switch (schedulerType) {
    case SchedulerType::Plaintext:
      return scheduler::createArithmeticPlaintextScheduler<unsafe>;
    case SchedulerType::NetworkPlaintext:
      return scheduler::createArithmeticNetworkPlaintextScheduler<unsafe>;
    case SchedulerType::Eager:
      return scheduler::createArithmeticEagerSchedulerWithInsecureEngine<
          unsafe>;
    case SchedulerType::Lazy:
      return scheduler::createArithmeticLazySchedulerWithInsecureEngine<unsafe>;
  }
}

const bool unsafe = true;

template <int schedulerId0, int schedulerId1>
void setupRealBackend(
    engine::communication::IPartyCommunicationAgentFactory& factory0,
    engine::communication::IPartyCommunicationAgentFactory& factory1) {
  auto task0 =
      [](std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory> factory) {
        scheduler::SchedulerKeeper<schedulerId0>::setScheduler(
            scheduler::createEagerSchedulerWithInsecureEngine<unsafe>(
                0, factory));
      };
  auto task1 =
      [](std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory> factory) {
        scheduler::SchedulerKeeper<schedulerId1>::setScheduler(
            scheduler::createEagerSchedulerWithInsecureEngine<unsafe>(
                1, factory));
      };

  auto future0 = std::async(
      task0,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(factory0));
  auto future1 = std::async(
      task1,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(factory1));
  future0.get();
  future1.get();
}

template <int schedulerId0, int schedulerId1>
void setupRealBackendWithLazyScheduler(
    engine::communication::IPartyCommunicationAgentFactory& factory0,
    engine::communication::IPartyCommunicationAgentFactory& factory1) {
  auto task0 =
      [](std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory> factory) {
        scheduler::SchedulerKeeper<schedulerId0>::setScheduler(
            scheduler::createLazySchedulerWithInsecureEngine<unsafe>(
                0, factory));
      };
  auto task1 =
      [](std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory> factory) {
        scheduler::SchedulerKeeper<schedulerId1>::setScheduler(
            scheduler::createLazySchedulerWithInsecureEngine<unsafe>(
                1, factory));
      };

  auto future0 = std::async(
      task0,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(factory0));
  auto future1 = std::async(
      task1,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(factory1));
  future0.get();
  future1.get();
}

} // namespace fbpcf
