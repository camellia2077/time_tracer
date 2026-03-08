// infrastructure/io/file_io_service.cpp
#if TT_ENABLE_CPP20_MODULES
import tracer.adapters.io.core.fs;
import tracer.adapters.io.utils.file_utils;
#endif

#include "infrastructure/io/file_io_service.hpp"

#if !TT_ENABLE_CPP20_MODULES
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"
#endif

#if TT_ENABLE_CPP20_MODULES
namespace modcore = tracer::adapters::io::modcore;
namespace modutils = tracer::adapters::io::modutils;
#else
namespace modcore {

using FileSystem = ::FileSystemHelper;

inline auto CreateDirectories(const std::filesystem::path& path) -> void {
  FileSystem::CreateDirectories(path);
}

}  // namespace modcore

namespace modutils {

[[nodiscard]] inline auto FindFilesByExtensionRecursively(
    const std::filesystem::path& root_path, const std::string& extension)
    -> std::vector<std::filesystem::path> {
  return ::FileUtils::FindFilesByExtensionRecursively(root_path, extension);
}

}  // namespace modutils
#endif

auto FileIoService::FindLogFiles(const std::filesystem::path& root_path)
    -> std::vector<std::filesystem::path> {
  return modutils::FindFilesByExtensionRecursively(root_path, ".txt");
}

void FileIoService::PrepareOutputDirectories(
    const std::filesystem::path& output_root) {
  modcore::CreateDirectories(output_root);
}
