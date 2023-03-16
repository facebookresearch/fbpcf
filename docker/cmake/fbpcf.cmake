# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)
include(${CMAKE_FOLDER}/cmake/enable_rdseed.cmake)
include(${CMAKE_FOLDER}/cmake/enable_threading.cmake)

find_package(gflags REQUIRED)

find_package(emp-ot)
find_package(emp-sh2pc REQUIRED)
include_directories(${EMP-SH2PC_INCLUDE_DIRS})
if(EMP_USE_RANDOM_DEVICE)
  ADD_DEFINITIONS(-DEMP_USE_RANDOM_DEVICE)
  message("EMP Use Random Device: on")
endif(EMP_USE_RANDOM_DEVICE)

find_package(folly REQUIRED)
set_and_check(FOLLY_INCLUDE_DIR /usr/local/include/folly)
set_and_check(FOLLY_CMAKE_DIR /usr/local/lib/cmake/folly)
if(NOT TARGET Folly::folly)
  include("${FOLLY_CMAKE_DIR}/folly-targets.cmake")
endif()
if(NOT folly_FIND_QUIETLY)
  message(STATUS "Found folly: ${PACKAGE_PREFIX_DIR}")
endif()

find_package(AWSSDK REQUIRED COMPONENTS s3)

find_package(google_cloud_cpp_storage REQUIRED)

find_library(re2 libre2.so)

# Find boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_compile_options(-lboost_serialization)

# since emp-tool is compiled with cc++11 and our games needs c++17 overwrite the
# compile option to c++17
add_compile_options(-std=c++17)
add_compile_options(-march=haswell)
