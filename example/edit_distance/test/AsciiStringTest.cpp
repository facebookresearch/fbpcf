/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../AsciiString.h" // @manual
#include <fbpcf/test/TestHelper.h>
#include <gtest/gtest.h>
#include <random>
#include <stdexcept>
#include "fbpcf/scheduler/PlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"

namespace fbpcf::edit_distance {

TEST(AsciiStringTest, testInputAndOutput) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecAsciiString = AsciiString<15, true, 0>;
  using PubAsciiString = AsciiString<15, false, 0>;
  using SecBatchAsciiString = AsciiString<15, true, 0, true>;
  using PubBatchAsciiString = AsciiString<15, false, 0, true>;

  std::string s1 = "Foo";
  std::string s2 = "Testing";

  PubAsciiString v1(s1);
  SecAsciiString v2(s2, 0);
  PubAsciiString v3 = v2.openToParty(0);

  std::vector<std::string> s3 = {"testing", "some", "words", "over", "here"};
  std::vector<std::string> s4 = {"Foo", "Bar", "Baz"};

  PubBatchAsciiString v5(s3);
  SecBatchAsciiString v4(s4, 0);
  PubBatchAsciiString v6 = v4.openToParty(0);

  SecAsciiString v7(v2.extractAsciiStringShare());
  SecBatchAsciiString v8(v4.extractAsciiStringShare());

  ASSERT_EQ(v1.getValue(), s1);
  ASSERT_EQ(v1.size(), s1.size());
  ASSERT_EQ(v1.at(0).getValue(), s1[0]);

  ASSERT_EQ(v2.openToParty(0).getValue(), s2);
  ASSERT_EQ(v3.size(), s2.size());

  testVectorEq(v5.getValue(), s3);
  testVectorEq(v5.size(), {7, 4, 5, 4, 4});

  testVectorEq(v6.getValue(), s4);
  testVectorEq(v6.size(), {3, 3, 3});

  ASSERT_EQ(v7.openToParty(0).getValue(), s2);
  testVectorEq(v8.openToParty(0).getValue(), s4);

  EXPECT_THROW(
      SecAsciiString("This string has too many letters in it", 0),
      std::runtime_error);
  EXPECT_THROW(
      PubAsciiString("This string has too many letters in it"),
      std::runtime_error);

  EXPECT_THROW(
      SecBatchAsciiString({"This string has too many letters in it"}, 0),
      std::runtime_error);
  EXPECT_THROW(
      PubBatchAsciiString({"This string has too many letters in it"}),
      std::runtime_error);
}

std::string randomString(
    std::uniform_int_distribution<char>& dist,
    std::mt19937_64& e,
    size_t len) {
  std::string rst;
  rst.reserve(len);
  for (size_t i = 0; i < len; i++) {
    rst += dist(e);
  }
  return rst;
}

TEST(AsciiStringTest, testToUpperCase) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecAsciiString = AsciiString<15, true, 0>;
  using PubAsciiString = AsciiString<15, false, 0>;
  using SecBatchAsciiString = AsciiString<15, true, 0, true>;
  using PubBatchAsciiString = AsciiString<15, false, 0, true>;

  size_t batchSize = 9;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist('a', 'z');

  for (size_t i = 0; i < 100; i++) {
    std::string s1 = randomString(dist, e, 12);
    std::string s2 = randomString(dist, e, 12);
    std::string s1ToUpper = s1;
    std::string s2ToUpper = s2;
    std::transform(
        s1ToUpper.begin(), s1ToUpper.end(), s1ToUpper.begin(), ::toupper);
    std::transform(
        s2ToUpper.begin(), s2ToUpper.end(), s2ToUpper.begin(), ::toupper);

    PubAsciiString pubStr1(s1);
    SecAsciiString secStr1(s2, 0);

    ASSERT_EQ(pubStr1.toUpperCase().getValue(), s1ToUpper);
    ASSERT_EQ(secStr1.toUpperCase().openToParty(0).getValue(), s2ToUpper);
  }

  for (size_t i = 0; i < 100; i++) {
    std::vector<std::string> s3(batchSize);
    std::vector<std::string> s4(batchSize);
    std::vector<std::string> s3ToUpper;
    std::vector<std::string> s4ToUpper;
    for (size_t j = 0; j < batchSize; j++) {
      s3[j] = randomString(dist, e, 12);
      s4[j] = randomString(dist, e, 12);
      auto str1 = s3[j];
      auto str2 = s4[j];
      std::transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
      std::transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
      s3ToUpper.push_back(str1);
      s4ToUpper.push_back(str2);
    }

    PubBatchAsciiString pubStr2(s3);
    SecBatchAsciiString secStr2(s4, 0);

    testVectorEq(pubStr2.toUpperCase().getValue(), s3ToUpper);
    testVectorEq(secStr2.toUpperCase().openToParty(0).getValue(), s4ToUpper);
  }
}

TEST(AsciiStringTest, testMux) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecAsciiString = AsciiString<15, true, 0>;
  using PubAsciiString = AsciiString<15, false, 0>;

  int partyId = 0;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist1('a', 'z');
  std::uniform_int_distribution<uint8_t> dist2(0, 1);

  for (size_t i = 0; i < 100; i++) {
    std::string v1 = randomString(dist1, e, 12);
    std::string v2 = randomString(dist1, e, 12);
    bool choice = dist2(e);

    SecAsciiString secStr1(v1, partyId);
    SecAsciiString secStr2(v2, partyId);
    PubAsciiString pubStr1(v1);
    PubAsciiString pubStr2(v2);

    frontend::Bit<true, 0> secChoice(choice, partyId);
    frontend::Bit<false, 0> pubChoice(choice);

    auto r1 = secStr1.mux(secChoice, secStr2);
    auto r2 = secStr1.mux(pubChoice, secStr2);
    auto r3 = secStr1.mux(secChoice, pubStr2);
    auto r4 = secStr1.mux(pubChoice, pubStr2);

    auto r5 = pubStr1.mux(secChoice, secStr2);
    auto r6 = pubStr1.mux(pubChoice, secStr2);
    auto r7 = pubStr1.mux(secChoice, pubStr2);
    auto r8 = pubStr1.mux(pubChoice, pubStr2);

    auto expectedValue = choice ? v2 : v1;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r3.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r4.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r5.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r6.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r7.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r8.getValue(), expectedValue);
  }
}

TEST(AsciiStringTest, testMuxBatch) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecBatchAsciiString = AsciiString<15, true, 0, true>;
  using PubBatchAsciiString = AsciiString<15, false, 0, true>;

  int partyId = 0;
  size_t batchSize = 9;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist1('a', 'z');
  std::uniform_int_distribution<uint8_t> dist2(0, 1);

  for (size_t i = 0; i < 100; i++) {
    std::vector<std::string> v1(batchSize);

    std::vector<std::string> v2(batchSize);
    std::vector<bool> choice(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      v1[j] = randomString(dist1, e, 12);
      v2[j] = randomString(dist1, e, 12);
      choice[j] = dist2(e);
    }

    SecBatchAsciiString secStr1(v1, partyId);
    SecBatchAsciiString secStr2(v2, partyId);
    PubBatchAsciiString pubStr1(v1);
    PubBatchAsciiString pubStr2(v2);

    frontend::Bit<true, 0, true> secChoice(choice, partyId);
    frontend::Bit<false, 0, true> pubChoice(choice);

    auto r1 = secStr1.mux(secChoice, secStr2);
    auto r2 = secStr1.mux(pubChoice, secStr2);
    auto r3 = secStr1.mux(secChoice, pubStr2);
    auto r4 = secStr1.mux(pubChoice, pubStr2);

    auto r5 = pubStr1.mux(secChoice, secStr2);
    auto r6 = pubStr1.mux(pubChoice, secStr2);
    auto r7 = pubStr1.mux(secChoice, pubStr2);
    auto r8 = pubStr1.mux(pubChoice, pubStr2);

    std::vector<std::string> expectedValue(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      expectedValue[j] = choice[j] ? v2[j] : v1[j];
    }

    testVectorEq(r1.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r2.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r3.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r4.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r5.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r6.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r7.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r8.getValue(), expectedValue);
  }
}

TEST(AsciiStringTest, testPrivateSize) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecAsciiString = AsciiString<15, true, 0>;

  int partyId = 0;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist1('a', 'z');
  std::uniform_int_distribution<size_t> strLength(1, 15);

  for (size_t i = 0; i < 100; i++) {
    auto len = strLength(e);
    std::string v1 = randomString(dist1, e, len);

    SecAsciiString secStr1(v1, partyId);

    auto r1 = secStr1.privateSize<16>();

    EXPECT_EQ(r1.openToParty(partyId).getValue(), v1.size());
  }
}

TEST(AsciiStringTest, testPrivateSizeBatch) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecBatchAsciiString = AsciiString<15, true, 0, true>;

  int partyId = 0;
  size_t batchSize = 9;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist1('a', 'z');
  std::uniform_int_distribution<size_t> strLength(1, 15);

  for (size_t i = 0; i < 100; i++) {
    std::vector<std::string> v1(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      auto len = strLength(e);
      v1[j] = randomString(dist1, e, len);
    }

    SecBatchAsciiString secStr1(v1, partyId);

    auto r1 = secStr1.privateSize<16>();

    std::vector<size_t> expectedValue(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      expectedValue[j] = v1[j].size();
    }

    testVectorEq(r1.openToParty(partyId).getValue(), expectedValue);
  }
}

TEST(AsciiStringTest, testBatchSize) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using SecBatchAsciiString = AsciiString<15, true, 0, true>;
  using PubBatchAsciiString = AsciiString<15, false, 0, true>;

  int partyId = 0;
  size_t batchSize = 9;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<char> dist1('a', 'z');
  std::uniform_int_distribution<uint8_t> dist2(0, 1);
  std::uniform_int_distribution<size_t> strLength(1, 15);

  std::vector<std::string> v1(batchSize);
  std::vector<bool> choice(batchSize);

  for (size_t j = 0; j < batchSize; j++) {
    auto len = strLength(e);
    v1[j] = randomString(dist1, e, len);
    choice[j] = dist2(e);
  }
  SecBatchAsciiString secStr1(v1, partyId);
  PubBatchAsciiString pubStr1(v1);

  frontend::Bit<true, 0, true> secChoice(choice, partyId);
  frontend::Bit<false, 0, true> pubChoice(choice);

  auto r1 = secStr1.mux(secChoice, secStr1);
  auto r2 = pubStr1.mux(secChoice, pubStr1);
  auto r3 = r1.privateSize<16>();

  EXPECT_EQ(secStr1.getBatchSize(), batchSize);
  EXPECT_EQ(pubStr1.getBatchSize(), batchSize);
  EXPECT_EQ(r1.getBatchSize(), batchSize);
  EXPECT_EQ(r2.getBatchSize(), batchSize);
  EXPECT_EQ(r3.getBatchSize(), batchSize);
}

} // namespace fbpcf::edit_distance
