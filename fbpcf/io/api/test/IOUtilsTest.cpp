/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/IOUtils.h"
#include <gtest/gtest.h>
#include <string>

namespace fbpcf::io {

TEST(IOUtilsTest, testIsCloudFile) {
  bool ans =
      IOUtils::isCloudFile("https://random_bucket.us-west-2.amazonaws.com");
  EXPECT_EQ(ans, true);

  ans = IOUtils::isCloudFile("/random/local/file");
  EXPECT_EQ(ans, false);
}

} // namespace fbpcf::io
