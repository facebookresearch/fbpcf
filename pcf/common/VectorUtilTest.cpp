#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "VectorUtil.h"

namespace pcf::vector {
TEST(FunctionalUtilTest, TestVectorAdd) {
  std::vector<int> v1{1, 2, 3};
  std::vector<int> v2{4, 5, 6};
  std::vector<int> expectedOutput{5, 7, 9};

  auto output = Add<int>(v1, v2);
  EXPECT_EQ(expectedOutput, output);
}

TEST(FunctionalUtilTest, TestVectorAddWithException) {
  std::vector<int> v1{1};
  std::vector<int> v2{2, 3};

  EXPECT_THROW(Add<int>(v1, v2), std::invalid_argument);
}
} // namespace pcf::vector
