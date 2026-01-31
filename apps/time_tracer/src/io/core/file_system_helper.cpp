// io/core/file_system_helper.cpp
#include "file_system_helper.hpp"

#include <stdexcept>

namespace fs = std::filesystem;

auto FileSystemHelper::exists(const fs::path& path) -> bool {
  return fs::exists(path);
}

auto FileSystemHelper::is_regular_file(const fs::path& path) -> bool {
  return fs::is_regular_file(path);
}

auto FileSystemHelper::is_directory(const fs::path& path) -> bool {
  return fs::is_directory(path);
}

void FileSystemHelper::create_directories(const fs::path& path) {
  try {
    fs::create_directories(path);
  } catch (const fs::filesystem_error& e) {
    // 将底层的文件系统错误转换为运行时错误，附带清晰的上下文信息
    throw std::runtime_error(
        "FileSystem Error: Failed to create directories at '" + path.string() +
        "'. Reason: " + e.what());
  }
}