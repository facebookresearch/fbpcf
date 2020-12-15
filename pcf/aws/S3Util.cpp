#include "S3Util.h"

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/Scheme.h>
#include <aws/s3/S3Client.h>

#include <folly/Format.h>
#include <folly/String.h>
#include <folly/Uri.h>

#include "../exception/AwsException.h"

namespace pcf::aws {
// format: https://bucket-name.s3.Region.amazonaws.com/key-name
S3ObjectReference uriToObjectReference(std::string url) {
  auto uri = folly::Uri(url);
  auto host = uri.host();
  auto path = uri.path();

  std::vector<std::string> vs;
  folly::split('.', host, vs);

  if (vs.size() != 5 || path.length() <= 1) {
    throw AwsException{folly::sformat("Incorrect S3 URI format: {}", url)};
  }

  // path.substr(1) to remove the first character '/'
  return S3ObjectReference{vs[2], vs[0], path.substr(1)};
}

std::unique_ptr<Aws::S3::S3Client> createS3Client(
    const S3ClientOption& option) {
  Aws::Client::ClientConfiguration config;

  if (option.region.has_value()) {
    config.region = option.region.value();
  } else if (std::getenv("AWS_DEFAULT_REGION")) {
    config.region = std::getenv("AWS_DEFAULT_REGION");
  }

  if (option.proxyHost.has_value()) {
    config.proxyHost = option.proxyHost.value();
  } else if (std::getenv("AWS_PROXY_HOST")) {
    config.proxyHost = std::getenv("AWS_PROXY_HOST");
  }

  if (option.proxyPort.has_value()) {
    config.proxyPort = option.proxyPort.value();
  } else if (std::getenv("AWS_PROXY_PORT")) {
    config.proxyPort = std::stoi(std::getenv("AWS_PROXY_PORT"));
  }

  if (option.proxyScheme.has_value()) {
    config.proxyScheme = option.proxyScheme.value() == "HTTPS"
        ? Aws::Http::Scheme::HTTPS
        : Aws::Http::Scheme::HTTP;
  } else if (std::getenv("AWS_PROXY_SCHEME")) {
    config.proxyScheme = std::string(std::getenv("AWS_PROXY_SCHEME")) == "HTTPS"
        ? Aws::Http::Scheme::HTTPS
        : Aws::Http::Scheme::HTTP;
  }

  if (option.proxySSLCertPath.has_value()) {
    config.proxySSLCertPath = option.proxySSLCertPath.value();
  } else if (std::getenv("AWS_PROXY_CERT_PATH")) {
    config.proxySSLCertPath = std::getenv("AWS_PROXY_CERT_PATH");
  }

  if (option.accessKeyId.has_value() && option.secretKey.has_value()) {
    Aws::Auth::AWSCredentials credentials(
        option.accessKeyId.value(), option.secretKey.value());
    return std::make_unique<Aws::S3::S3Client>(credentials, config);
  } else {
    return std::make_unique<Aws::S3::S3Client>(config);
  }
}
} // namespace pcf::aws
