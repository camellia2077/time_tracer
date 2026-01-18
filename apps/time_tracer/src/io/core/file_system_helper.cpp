// io/core/file_system_helper.cpp
#include "file_system_helper.hpp"
#include <stdexcept>

namespace fs = std::filesystem;

bool FileSystemHelper::exists(const fs::path& path) {
    return fs::exists(path);
}

bool FileSystemHelper::is_regular_file(const fs::path& path) {
    return fs::is_regular_file(path);
}

bool FileSystemHelper::is_directory(const fs::path& path) {
    return fs::is_directory(path);
}

void FileSystemHelper::create_directories(const fs::path& path) {
    try {
        fs::create_directories(path);
    } catch (const fs::filesystem_error& e) {
        // 将底层的文件系统错误转换为运行时错误，附带清晰的上下文信息
        throw std::runtime_error("FileSystem Error: Failed to create directories at '" + path.string() + "'. Reason: " + e.what());
    }
}