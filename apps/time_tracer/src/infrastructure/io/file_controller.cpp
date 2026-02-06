// infrastructure/io/file_controller.cpp
#include "infrastructure/io/file_controller.hpp"

#include <iostream>

#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"

auto FileController::FindLogFiles(const std::filesystem::path& root_path)
    -> std::vector<std::filesystem::path> {
  // 纯粹的文件查找逻辑
  return FileUtils::FindFilesByExtensionRecursively(root_path, ".txt");
}

void FileController::PrepareOutputDirectories(
    const std::filesystem::path& output_root,
    const std::filesystem::path& export_root) {
  // 委托给 FileSystemHelper，它会处理异常和错误包装
  FileSystemHelper::CreateDirectories(output_root);
  FileSystemHelper::CreateDirectories(output_root / "data");
  FileSystemHelper::CreateDirectories(output_root / "db");
  FileSystemHelper::CreateDirectories(export_root);
}
