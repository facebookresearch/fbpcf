# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.10.1)
project(fbpcf)
set(NAME "fbpcf")
set(EDIT_DISTANCE_NAME "fbpcf_edit_distance")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't compile with AVX512 instructions since many of the AWS
# instances won't have access to that instruction set.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-avx512f")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-avx512f")

include(cmake/fbpcf.cmake)
install(DIRECTORY cmake/ DESTINATION cmake/)
find_package(Boost REQUIRED COMPONENTS serialization)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# Build fbpcf static library
file(GLOB_RECURSE fbpcf_src
  fbpcf/**.c
  fbpcf/**.cpp
  fbpcf/**.h
  fbpcf/**.hpp)
list(FILTER fbpcf_src EXCLUDE REGEX ".*\/test\/.*") # Exclude test files

add_library(${NAME} STATIC
  ${fbpcf_src})
target_link_libraries(
  fbpcf
  INTERFACE
  ${AWSSDK_LINK_LIBRARIES}
  ${EMP-OT_LIBRARIES}
  google-cloud-cpp::storage
  Folly::folly
  Boost::serialization
  ${Boost_LIBRARIES}
  re2)

# Build Edit Distance Example Library
file(GLOB_RECURSE edit_distance_src
  example/edit_distance/**.c
  example/edit_distance/**.cpp
  example/edit_distance/**.h
  example/edit_distance/**.hpp)
list(FILTER edit_distance_src EXCLUDE REGEX ".*\/test\/.*") # Exclude test files
list(FILTER edit_distance_src EXCLUDE REGEX ".*\/[m|M]ain\/.*") # Exclude main

add_library(${EDIT_DISTANCE_NAME} STATIC
  ${edit_distance_src})
target_link_libraries(
  fbpcf_edit_distance
  INTERFACE
  fbpcf
)

install(DIRECTORY fbpcf/ DESTINATION include/fbpcf/)
install(TARGETS ${NAME} DESTINATION lib)
install(TARGETS ${EDIT_DISTANCE_NAME} DESTINATION lib)

add_executable(
  millionaire
  "example/millionaire/main.cpp"
  "example/millionaire/MillionaireApp.h"
  "example/millionaire/MillionaireApp.cpp"
  "example/millionaire/MillionaireGame.h")
target_link_libraries(
  millionaire
  fbpcf)

add_executable(
  billionaire
  "example/billionaire_problem/main.cpp"
  "example/billionaire_problem/BillionaireProblemGame.h"
  "example/billionaire_problem/BillionaireProblemGame_impl.h")
target_link_libraries(
  billionaire
  fbpcf)

add_executable(
  edit_distance_runner
  "example/edit_distance/main.cpp"
  "example/edit_distance/MainUtil.h")
target_link_libraries(
  edit_distance_runner
  fbpcf
  fbpcf_edit_distance)

add_executable(
  edit_distance_validator
  "example/edit_distance/validatorMain.cpp")
target_link_libraries(
  edit_distance_validator
  fbpcf
  fbpcf_edit_distance)

install(TARGETS millionaire DESTINATION bin)
install(TARGETS billionaire DESTINATION bin)
install(TARGETS edit_distance_runner DESTINATION bin)
install(TARGETS edit_distance_validator DESTINATION bin)
