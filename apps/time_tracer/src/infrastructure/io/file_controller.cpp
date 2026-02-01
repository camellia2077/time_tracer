// infrastructure/io/file_controller.cpp
#include "file_controller.hpp"

#include <iostream>

#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"

auto FileController::find_log_files(const std::filesystem::path& root_path)
    -> std::vector<std::filesystem::path> {
  // 纯粹的文件查找逻辑
  return FileUtils::find_files_by_extension_recursively(root_path, ".txt");
}

void FileController::prepare_output_directories(
    const std::filesystem::path& output_root,
    const std::filesystem::path& export_root) {
  // 委托给 FileSystemHelper，它会处理异常和错误包装
  FileSystemHelper::create_directories(output_root);
  FileSystemHelper::create_directories(output_root / "data");
  FileSystemHelper::create_directories(output_root / "db");
  FileSystemHelper::create_directories(export_root);
}
