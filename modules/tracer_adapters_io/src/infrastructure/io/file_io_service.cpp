// infrastructure/io/file_io_service.cpp
#include "infrastructure/io/file_io_service.hpp"

#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"

auto FileIoService::FindLogFiles(const std::filesystem::path& root_path)
    -> std::vector<std::filesystem::path> {
  return FileUtils::FindFilesByExtensionRecursively(root_path, ".txt");
}

void FileIoService::PrepareOutputDirectories(
    const std::filesystem::path& output_root) {
  FileSystemHelper::CreateDirectories(output_root);
}
