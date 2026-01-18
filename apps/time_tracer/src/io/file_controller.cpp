// io/file_controller.cpp
#include "file_controller.hpp"
#include "io/utils/file_utils.hpp"
#include "io/core/file_system_helper.hpp"
#include <iostream>

std::vector<std::filesystem::path> FileController::find_log_files(const std::filesystem::path& root_path) const {
    // 纯粹的文件查找逻辑
    return FileUtils::find_files_by_extension_recursively(root_path, ".txt");
}

void FileController::prepare_output_directories(const std::filesystem::path& output_root, const std::filesystem::path& export_root) const {
    // 委托给 FileSystemHelper，它会处理异常和错误包装
    FileSystemHelper::create_directories(output_root);
    FileSystemHelper::create_directories(export_root);
}