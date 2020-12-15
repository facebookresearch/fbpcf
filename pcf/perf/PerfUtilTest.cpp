#include <gtest/gtest.h>

#include "PerfUtil.h"

namespace pcf::perf {
class TestClass {
 public:
  int a(int x, int y) {
    return x + y;
  }

  void b(int /* x */, int /* y */) {}

  int c() {
    return 3;
  }

  void d() {
    return;
  }
};

TEST(PerfUtilTest, TestA) {
  auto decorator = decorate(
      "TestClass::a",
      static_cast<std::function<int(TestClass*, int, int)>>(&TestClass::a));

  TestClass a;
  EXPECT_EQ(3, decorator(&a, 1, 2));
}

TEST(PerfUtilTest, TestB) {
  auto decorator = decorate(
      "TestClass::b",
      static_cast<std::function<void(TestClass*, int, int)>>(&TestClass::b));

  TestClass b;
  decorator(&b, 1, 2);
}

TEST(PerfUtilTest, TestC) {
  auto decorator = decorate(
      "TestClass::c",
      static_cast<std::function<int(TestClass*)>>(&TestClass::c));

  TestClass c;
  EXPECT_EQ(3, decorator(&c));
}

TEST(PerfUtilTest, TestD) {
  auto decorator = decorate(
      "TestClass::d",
      static_cast<std::function<void(TestClass*)>>(&TestClass::d));

  TestClass d;
  decorator(&d);
}
} // namespace pcf::perf
