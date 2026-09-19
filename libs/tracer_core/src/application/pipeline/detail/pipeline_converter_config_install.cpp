#include "application/pipeline/detail/pipeline_converter_config_install.hpp"

#include <stdexcept>
#include <string>

namespace tracer::core::application::pipeline::detail {

auto ResolveConverterMainConfigPath(
    const std::filesystem::path& main_config_path) -> std::filesystem::path {
  if (main_config_path.empty()) {
    throw std::invalid_argument(
        "Converter main config path must not be empty.");
  }
  return std::filesystem::absolute(main_config_path);
}

auto EnsureConverterConfigSourceExists(const std::filesystem::path& path,
                                       std::string_view label) -> void {
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error(std::string(label) +
                             " must be an existing file: " + path.string());
  }
}

auto RemoveConverterUserConfigDirectory(
    const std::filesystem::path& target_root) -> void {
  const std::filesystem::path kUserConfigDir = target_root / "user";
  if (!std::filesystem::exists(kUserConfigDir)) {
    return;
  }

  std::error_code io_error;
  std::filesystem::remove_all(kUserConfigDir, io_error);
  if (io_error) {
    throw std::runtime_error(
        "Failed to remove user config directory: " + kUserConfigDir.string() +
        " | " + io_error.message());
  }
}

auto CopyConverterUserConfigDirectory(const std::filesystem::path& source_root,
                                      const std::filesystem::path& target_root)
    -> void {
  if (!std::filesystem::exists(source_root) ||
      !std::filesystem::is_directory(source_root)) {
    throw std::runtime_error("User config source must be a directory: " +
                             source_root.string());
  }

  // Install/import treats converter config as a small text bundle, so full
  // replacement keeps the active user config an exact mirror of the package.
  std::error_code io_error;
  std::filesystem::create_directories(target_root, io_error);
  if (io_error) {
    throw std::runtime_error(
        "Failed to prepare user config target directory: " +
        target_root.string() + " | " + io_error.message());
  }

  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(source_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const auto kRelativePath =
        std::filesystem::relative(entry.path(), source_root, io_error);
    if (io_error) {
      throw std::runtime_error(
          "Failed to resolve user config child path: " + entry.path().string() +
          " | " + io_error.message());
    }
    const std::filesystem::path kTargetPath = target_root / kRelativePath;
    std::filesystem::create_directories(kTargetPath.parent_path(), io_error);
    if (io_error) {
      throw std::runtime_error("Failed to prepare user config child target: " +
                               kTargetPath.string() + " | " +
                               io_error.message());
    }
    std::filesystem::copy_file(
        entry.path(), kTargetPath,
        std::filesystem::copy_options::overwrite_existing, io_error);
    if (io_error) {
      throw std::runtime_error(
          "Failed to copy user config child: " + entry.path().string() +
          " -> " + kTargetPath.string() + " | " + io_error.message());
    }
  }
}

}  // namespace tracer::core::application::pipeline::detail
