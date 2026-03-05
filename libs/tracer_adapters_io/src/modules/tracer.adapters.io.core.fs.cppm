module;

#include "infrastructure/io/core/file_system_helper.hpp"

export module tracer.adapters.io.core.fs;

export namespace tracer::adapters::io::modcore {

using FileSystem = ::FileSystemHelper;

[[nodiscard]] inline auto Exists(const std::filesystem::path& path) -> bool {
  return FileSystem::Exists(path);
}

[[nodiscard]] inline auto IsRegularFile(const std::filesystem::path& path)
    -> bool {
  return FileSystem::IsRegularFile(path);
}

[[nodiscard]] inline auto IsDirectory(const std::filesystem::path& path)
    -> bool {
  return FileSystem::IsDirectory(path);
}

inline auto CreateDirectories(const std::filesystem::path& path) -> void {
  FileSystem::CreateDirectories(path);
}

}  // namespace tracer::adapters::io::modcore
