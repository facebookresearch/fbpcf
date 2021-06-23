/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <memory>
#include <string>

#include "IFileManager.h"
#include "IInputStream.h"

namespace fbpcf::io {
enum class FileType { Local, S3 };

std::unique_ptr<IInputStream> getInputStream(const std::string& fileName);

std::string read(const std::string& fileName);

void write(const std::string& fileName, const std::string& data);

FileType getFileType(const std::string& fileName);

std::unique_ptr<fbpcf::IFileManager> getFileManager(const std::string& fileName);
} // namespace fbpcf::io
