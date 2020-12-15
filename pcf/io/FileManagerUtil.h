#pragma once

#include <memory>
#include <string>

#include "IFileManager.h"
#include "IInputStream.h"

namespace pcf::io {
enum class FileType { Local, S3 };

std::unique_ptr<IInputStream> getInputStream(const std::string& fileName);

std::string read(const std::string& fileName);

void write(const std::string& fileName, const std::string& data);

FileType getFileType(const std::string& fileName);

std::unique_ptr<pcf::IFileManager> getFileManager(const std::string& fileName);
} // namespace pcf::io
