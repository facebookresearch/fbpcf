# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.10.1)
project(private-lift)
set(NAME "private-lift")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)

find_package(emp-sh2pc REQUIRED)
include_directories(${EMP-SH2PC_INCLUDE_DIRS})

if(EMP_USE_RANDOM_DEVICE)
  ADD_DEFINITIONS(-DEMP_USE_RANDOM_DEVICE)
  message("EMP Use Random Device: on")
endif(EMP_USE_RANDOM_DEVICE)

find_package(folly REQUIRED)
find_package(gflags REQUIRED)

set_and_check(FOLLY_INCLUDE_DIR /usr/local/include/folly)
set_and_check(FOLLY_CMAKE_DIR /usr/local/lib/cmake/folly)
if(NOT TARGET Folly::folly)
  include("${FOLLY_CMAKE_DIR}/folly-targets.cmake")
endif()

set(FOLLY_LIBRARIES Folly::folly)

if(NOT folly_FIND_QUIETLY)
  message(STATUS "Found folly: ${PACKAGE_PREFIX_DIR}")
endif()

find_package(AWSSDK REQUIRED COMPONENTS s3)

# since emp-tool is compiled with cc++11 and our games needs c++17 overwrite the
# compile option to c++17
add_compile_options(-std=c++17)

# TODO: not excluding Test files
file(GLOB pcf_io_sources pcf/io/**[!Test].cpp pcf/io/**.c
     pcf/io/**.hpp pcf/io/**.h)

file(GLOB pcf_aws_sources pcf/aws/**[!Test].cpp pcf/aws/**.c
     pcf/aws/**.hpp pcf/aws/**.h)

file(GLOB pcf_common_sources pcf/common/**[!Test].cpp pcf/common/**.c
     pcf/common/**.hpp pcf/common/**.h)

file(GLOB pcf_exception_sources pcf/exception/**.cpp
     pcf/exception/**.c pcf/exception/**.hpp
     pcf/exception/**.h)

file(GLOB pcf_mpc_sources pcf/mpc/**[!Test].cpp pcf/mpc/**.c
     pcf/mpc/**.hpp pcf/mpc/**.h)

file(GLOB pcf_system_sources pcf/system/**[!Test].cpp pcf/system/**.c
     pcf/system/**.hpp pcf/system/**.h)


# build pcf as a library
add_library(fbpcf STATIC ${pcf_io_sources}
  ${pcf_aws_sources}
  ${pcf_common_sources}
  ${pcf_exception_sources}
  ${pcf_mpc_sources}
  ${pcf_system_sources})
target_link_libraries(
  fbpcf INTERFACE ${AWSSDK_LINK_LIBRARIES} ${FOLLY_LIBRARIES})


# private lift executables
add_executable(
  calculator
  "lift/calculator/main.cpp"
  "lift/calculator/OutputMetrics.hpp"
  "lift/common/GroupedLiftMetrics.h"
  "lift/common/GroupedLiftMetrics.cpp"
  "lift/common/LiftMetrics.h"
  "lift/common/LiftMetrics.cpp"
  "lift/calculator/CalculatorApp.h"
  "lift/calculator/CalculatorApp.cpp"
  "lift/calculator/CalculatorGame.h"
  "lift/calculator/OutputMetrics.h"
  "lift/calculator/InputData.cpp"
  "lift/calculator/InputData.h"
  "lift/calculator/CalculatorGameConfig.h"
  "lift/calculator/OutputMetricsData.h"
  "lift/common/PrivateData.h"
  "lift/common/SecretSharing.h"
  "lift/common/EmpOperationUtil.h"
  "lift/common/Csv.h"
  "lift/common/Csv.cpp")
target_link_libraries(
  calculator
  ${FOLLY_LIBRARIES}
  ${EMP-OT_LIBRARIES}
  fbpcf
  stdc++fs
  libre2.a)

add_executable(
  aggregator
  "lift/aggregator/main_kanon.cpp"
  "lift/common/GroupedLiftMetrics.h"
  "lift/common/GroupedLiftMetrics.cpp"
  "lift/common/LiftMetrics.h"
  "lift/common/LiftMetrics.cpp"
  "lift/aggregator/KAnonymityAggregatorApp.h"
  "lift/aggregator/KAnonymityAggregatorApp.cpp"
  "lift/aggregator/MetricsMapper.h"
  "lift/aggregator/MetricsMapper.cpp"
  "lift/aggregator/KAnonymityLiftAggregationGame.h")
target_link_libraries(
  aggregator
  ${FOLLY_LIBRARIES}
  ${EMP-OT_LIBRARIES}
  fbpcf
  stdc++fs
  libre2.a)

add_executable(
  millionaire
  "example/millionaire/main.cpp"
  "example/millionaire/MillionaireApp.h"
  "example/millionaire/MillionaireApp.cpp"
  "example/millionaire/MillionaireGame.h")
target_link_libraries(
  millionaire
  ${FOLLY_LIBRARIES}
  ${EMP-OT_LIBRARIES}
  fbpcf)
