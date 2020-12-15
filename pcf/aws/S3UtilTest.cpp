#include "S3Util.h"

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "../exception/AwsException.h"

namespace pcf {
TEST(S3Util, uriToObjectReference) {
  auto uri = "https://bucket.s3.region.amazonaws.com/key";
  auto ref = pcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("key", ref.key);
}

TEST(S3Util, uriToObjectReference_Subfolder) {
  auto uri = "https://bucket.s3.region.amazonaws.com/folder/key";
  auto ref = pcf::aws::uriToObjectReference(uri);

  EXPECT_EQ("region", ref.region);
  EXPECT_EQ("bucket", ref.bucket);
  EXPECT_EQ("folder/key", ref.key);
}

TEST(S3Util, uriToObjectReferenceException_1) {
  auto uri = "https://bucket.s3.region.amazonaws.com/";
  EXPECT_THROW(pcf::aws::uriToObjectReference(uri), AwsException);
}

TEST(S3Util, uriToObjectReferenceException_2) {
  auto uri = "https://bucket.s3.region.com/key";
  EXPECT_THROW(pcf::aws::uriToObjectReference(uri), AwsException);
}
} // namespace pcf
