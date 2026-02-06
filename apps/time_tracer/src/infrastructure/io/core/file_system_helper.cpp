// infrastructure/io/core/file_system_helper.cpp
#include "infrastructure/io/core/file_system_helper.hpp"

#include <stdexcept>

namespace fs = std::filesystem;

auto FileSystemHelper::Exists(const fs::path& path) -> bool {
  return fs::exists(path);
}

auto FileSystemHelper::IsRegularFile(const fs::path& path) -> bool {
  return fs::is_regular_file(path);
}

auto FileSystemHelper::IsDirectory(const fs::path& path) -> bool {
  return fs::is_directory(path);
}

auto FileSystemHelper::CreateDirectories(const fs::path& path) -> void {
  try {
    fs::create_directories(path);
  } catch (const fs::filesystem_error& e) {
    // 将底层的文件系统错误转换为运行时错误，附带清晰的上下文信息
    throw std::runtime_error(
        "FileSystem Error: Failed to create directories at '" + path.string() +
        "'. Reason: " + e.what());
  }
}
