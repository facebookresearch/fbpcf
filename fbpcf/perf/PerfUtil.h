/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <chrono>
#include <functional>

#include "folly/logging/xlog.h"

namespace fbpcf::perf {

template <typename... Args>
std::function<void(Args...)> decorate(
    const std::string& fName,
    const std::function<void(Args...)>& f) {
  return [fName, f](Args... args) {
    auto start = std::chrono::high_resolution_clock::now();

    f(std::forward<Args>(args)...);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    XLOGF(INFO, "{} execution time (ms): {}", fName, duration.count());
  };
}

template <typename R, typename... Args>
std::function<R(Args...)> decorate(
    const std::string& fName,
    const std::function<R(Args...)>& f) {
  return [fName, f](Args... args) {
    auto start = std::chrono::high_resolution_clock::now();

    auto res = f(std::forward<Args>(args)...);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    XLOGF(INFO, "{} execution time (ms): {}", fName, duration.count());
    return res;
  };
}
} // namespace fbpcf::perf
