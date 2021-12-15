# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# GCP Testing
add_executable(
  gcp_test
  "fbpcf/gcp/test/GCSUtilTest.cpp"
)
target_link_libraries(
  gcp_test
  fbpcf
  gtest_main
)
include(GoogleTest)
gtest_discover_tests(gcp_test)
install(TARGETS gcp_test DESTINATION test)
