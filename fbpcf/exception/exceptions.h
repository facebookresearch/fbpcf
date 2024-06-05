/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>

namespace common {

/**
 * PCF exceptions
 * --------------
 */
namespace exceptions {

/**
 * When to use:
 *   - While parsing text blobs, CSVs, json
 *    - For example: while parsing CSV if we see more number of columns in a row
 *          than the header.
 */
class ParseError : virtual public std::exception {
 public:
  explicit ParseError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - While constructing an object with incorrect/un-supported argument
 * value/type
 *    - For example:
 * class AggMetrics{
 *   explicit AggMetrics(AggMetricType type) : type_{type} {
 *     if (type_ == AggMetricType::kDict) {
 *       val_ = MetricsDict{};
 *     } else if (type_ == AggMetricType::kList) {
 *       val_ = MetricsList{};
 *     } else if (type_ == AggMetricType::kValue) {
 *       val_ = MetricsValue{0};
 *     } else {
 *       XLOG(ERR) << "Construction not supported for:"
 *                 << \
 *                static_cast<std::underlying_type<AggMetricType>::type>(type);
 *       throw common::ConstructionError("Contructor only supports one of kList,
 *            kDict or kValue");
 *     }
 *   }
 * };
 */
class ConstructionError : virtual public std::exception {
 public:
  explicit ConstructionError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - When any dynamic obj fails to meet the type requirement.
 *    - Example: folly::dynamic expects folly::OBJECT but got folly::INT
 *      instead.
 */
class RuntimeTypeError : virtual public std::exception {
 public:
  explicit RuntimeTypeError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - While validating if a parsed json object adheres to schema previously
 * agreed upon
 *    - Example: ShardValidator<ShardSchemaType::kAdObjFormat> object will
 *      throw this exception if the AggMetric does not follow this format.
 */
class SchemaTraceError : virtual public std::exception {
 public:
  explicit SchemaTraceError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - similar situation NotImplementedError in python, raise this exception
 *    while class is still being developed.
 */
class NotImplementedError : virtual public std::exception {
 public:
  explicit NotImplementedError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - throw when a method/procedure is/will no longer be supported.
 */
class NotSupportedError : virtual public std::exception {
 public:
  explicit NotSupportedError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - When we are trying to access an illegal variable.
 *    - Example: when a AggMetric<AggMetricType::kPlaintext> object tries to
 *      access getSecValueXor().
 */
class InvalidAccessError : virtual public std::exception {
 public:
  explicit InvalidAccessError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

/**
 * When to use:
 *  - When the connections fail to establish between the parties.
 */
class ConnectionError : virtual public std::exception {
 public:
  explicit ConnectionError(std::string msg) : msg_{msg} {}

  const char* what() const noexcept override {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

} // namespace exceptions
} // namespace common
