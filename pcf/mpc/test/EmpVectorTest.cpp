#include <memory>

#include <gtest/gtest.h>

#include "../../common/FunctionalUtil.h"
#include "../../exception/PcfException.h"
#include "../../system/CpuUtil.h"
#include "../EmpGame.h"
#include "../EmpTestUtil.h"
#include "../EmpVector.h"
#include "../QueueIO.h"

namespace pcf {
class EmpVectorTest
    : public EmpGame<QueueIO, std::vector<int64_t>, std::vector<int64_t>> {
 public:
  EmpVectorTest(std::unique_ptr<QueueIO> io, Party party)
      : EmpGame(std::move(io), party) {}

  std::vector<int64_t> play(const std::vector<int64_t>& input) override {
    EmpVector<emp::Integer> v;

    v.add(input);

    auto res = v.map([](emp::Integer x, emp::Integer y) { return x ^ y; });
    return pcf::functional::map<emp::Integer, int64_t>(
        res, [](emp::Integer i) { return i.reveal<int64_t>(emp::PUBLIC); });
  }
};

TEST(EmpVectorTest, AddIntegerWithException) {
  EmpVector<emp::Integer> v;
  EXPECT_THROW(v.add(true), PcfException);
}

TEST(EmpVectorTest, AddBitWithException) {
  EmpVector<emp::Bit> v;
  EXPECT_THROW(v.add(1), PcfException);
}

TEST(EmpVectorTest, TestMap) {
  if (!pcf::mpc::isTestable()) {
    GTEST_SKIP();
  }

  std::vector<int64_t> expectedRes{0, 0, 0};

  auto res =
      pcf::mpc::test<EmpVectorTest, std::vector<int64_t>, std::vector<int64_t>>(
          std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{1, 2, 3});

  EXPECT_EQ(expectedRes, res.first);
  EXPECT_EQ(expectedRes, res.second);
}
} // namespace pcf
