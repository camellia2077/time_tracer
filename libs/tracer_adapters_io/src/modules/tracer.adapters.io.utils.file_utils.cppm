module;

#include "infra/io/utils/file_utils.hpp"

export module tracer.adapters.io.utils.file_utils;

export namespace tracer::adapters::io::modutils {

[[nodiscard]] inline auto FindFilesByExtensionRecursively(
    const std::filesystem::path& root_path, const std::string& extension)
    -> std::vector<std::filesystem::path> {
  return ::FileUtils::FindFilesByExtensionRecursively(root_path, extension);
}

[[nodiscard]] inline auto ResolveFiles(
    const std::vector<std::string>& input_paths, const std::string& extension)
    -> std::vector<std::string> {
  return ::FileUtils::ResolveFiles(input_paths, extension);
}

}  // namespace tracer::adapters::io::modutils
