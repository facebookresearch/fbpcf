/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/exception/GcpException.h"
#include "fbpcf/gcp/MockGCSClient.h"
#include "fbpcf/io/GCSFileManager.cpp" // imported for linking template class
#include "fbpcf/io/GCSFileManager.h"
#include "fbpcf/io/MockFileManager.h"

using ::testing::_;

const std::string GCSUrl = "https://storage.cloud.google.com/bucket/key";

namespace fbpcf {
TEST(GCSFileManager, testReadWithException) {
  auto GCSClient = std::make_shared<MockGCSClient>();
  EXPECT_CALL(*GCSClient, ReadObject(_, _, _)).Times(1);
  GCSFileManager<MockGCSClient> fileManager{std::move(GCSClient)};

  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.read(GCSUrl), GcpException);
}

TEST(GCSFileManager, testWriteWithException) {
  auto GCSClient = std::make_shared<MockGCSClient>();
  EXPECT_CALL(*GCSClient, WriteObject(_, _)).Times(1);
  GCSFileManager<MockGCSClient> fileManager{std::move(GCSClient)};
  // by default, the call will fail, because the response indicates failure
  EXPECT_THROW(fileManager.write(GCSUrl, "ABC"), GcpException);
}
} // namespace fbpcf
