# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.10.1)
project(fbpcf)
set(NAME "fbpcf")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/fbpcf.cmake)
install(DIRECTORY cmake/ DESTINATION cmake/)

# Build fbpcf static library
file(GLOB_RECURSE fbpcf_src
  fbpcf/**.c
  fbpcf/**.cpp
  fbpcf/**.h
  fbpcf/**.hpp)
list(FILTER fbpcf_src EXCLUDE REGEX ".*\/test\/.*") # Exclude test files
list(FILTER fbpcf_src EXCLUDE REGEX ".*\/example\/.*") # Exclude example files

add_library(${NAME} STATIC
  ${fbpcf_src})
target_link_libraries(
  fbpcf
  INTERFACE
  ${AWSSDK_LINK_LIBRARIES}
  ${EMP-OT_LIBRARIES}
  Folly::folly
  re2)

install(DIRECTORY fbpcf/ DESTINATION include/fbpcf/)
install(TARGETS ${NAME} DESTINATION lib)

add_executable(
  millionaire
  "example/millionaire/main.cpp"
  "example/millionaire/MillionaireApp.h"
  "example/millionaire/MillionaireApp.cpp"
  "example/millionaire/MillionaireGame.h")
target_link_libraries(
  millionaire
  fbpcf)

install(TARGETS millionaire DESTINATION bin)
