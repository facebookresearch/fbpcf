/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/engine/tuple_generator/ArithmeticTupleGenerator.h"
#include "fbpcf/engine/tuple_generator/IArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator {

class ArithmeticTupleGeneratorFactory
    : public IArithmeticTupleGeneratorFactory {
 public:
  ArithmeticTupleGeneratorFactory(
      std::unique_ptr<IProductShareGeneratorFactory> productShareFactory,
      std::unique_ptr<util::IPrgFactory> prgFactory,
      int bufferSize,
      int myId,
      int numberOfParty)
      : productShareFactory_(std::move(productShareFactory)),
        prgFactory_(std::move(prgFactory)),
        bufferSize_(bufferSize),
        myId_(myId),
        numberOfParty_(numberOfParty) {}

  /**
   * Create real tuple generator;
   */
  std::unique_ptr<IArithmeticTupleGenerator> create() override {
    std::map<int, std::unique_ptr<IProductShareGenerator>>
        productShareGeneratorMap;
    for (int i = 0; i < numberOfParty_; i++) {
      if (i != myId_) {
        productShareGeneratorMap.emplace(i, productShareFactory_->create(i));
      }
    }
    return std::make_unique<ArithmeticTupleGenerator>(
        std::move(productShareGeneratorMap),
        prgFactory_->create(util::getRandomM128iFromSystemNoise()),
        bufferSize_);
  }

 private:
  std::unique_ptr<IProductShareGeneratorFactory> productShareFactory_;
  std::unique_ptr<util::IPrgFactory> prgFactory_;
  int bufferSize_;
  int myId_;
  int numberOfParty_;
};

} // namespace fbpcf::engine::tuple_generator
