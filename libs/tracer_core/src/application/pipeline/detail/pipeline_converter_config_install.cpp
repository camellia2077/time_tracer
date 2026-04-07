#include "application/pipeline/detail/pipeline_converter_config_install.hpp"

#include <stdexcept>
#include <string>

namespace tracer::core::application::pipeline::detail {

auto ResolveConverterConfigPathSet(const std::filesystem::path& main_config_path)
    -> ConverterConfigPathSet {
  if (main_config_path.empty()) {
    throw std::invalid_argument(
        "Converter main config path must not be empty.");
  }

  const std::filesystem::path kResolvedMainConfigPath =
      std::filesystem::absolute(main_config_path);
  const std::filesystem::path kConfigDir = kResolvedMainConfigPath.parent_path();
  return {
      .main_config_path = kResolvedMainConfigPath,
      .alias_mapping_path = kConfigDir / "alias_mapping.toml",
      .duration_rules_path = kConfigDir / "duration_rules.toml",
      // Keep application-layer install logic filesystem-only. The index is
      // copied as-is, and infra config loader owns how that index is parsed.
      .alias_directory_path = kConfigDir / "aliases",
  };
}

auto EnsureConverterConfigSourceExists(const std::filesystem::path& path,
                                       std::string_view label) -> void {
  if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error(std::string(label) +
                             " must be an existing file: " + path.string());
  }
}

auto CopyConverterConfigFile(const std::filesystem::path& source_path,
                             const std::filesystem::path& target_path,
                             std::string_view label) -> void {
  std::error_code kIoError;
  std::filesystem::create_directories(target_path.parent_path(), kIoError);
  if (kIoError) {
    throw std::runtime_error("Failed to prepare " + std::string(label) +
                             " target directory: " + target_path.string() +
                             " | " + kIoError.message());
  }

  std::filesystem::copy_file(
      source_path, target_path, std::filesystem::copy_options::overwrite_existing,
      kIoError);
  if (kIoError) {
    throw std::runtime_error("Failed to install " + std::string(label) + ": " +
                             source_path.string() + " -> " +
                             target_path.string() + " | " + kIoError.message());
  }
}

auto RemoveConverterAliasDirectory(const std::filesystem::path& target_root)
    -> void {
  const std::filesystem::path kAliasDir = target_root / "aliases";
  if (!std::filesystem::exists(kAliasDir)) {
    return;
  }

  std::error_code io_error;
  std::filesystem::remove_all(kAliasDir, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to remove converter alias directory: " +
                             kAliasDir.string() + " | " +
                             io_error.message());
  }
}

auto CopyConverterAliasDirectory(const std::filesystem::path& source_root,
                                 const std::filesystem::path& target_root)
    -> void {
  if (!std::filesystem::exists(source_root)) {
    return;
  }
  if (!std::filesystem::is_directory(source_root)) {
    throw std::runtime_error("Alias config source must be a directory: " +
                             source_root.string());
  }

  // Install/import treats converter config as a small text bundle, not as a
  // high-volume dataset. Because the files are small and low-frequency, full
  // replacement is preferred over incremental diff/merge logic.
  //
  // Child alias files are therefore copied as a whole directory bundle so the
  // active config remains an exact mirror of the source config without stale
  // leftovers from older alias files.
  std::error_code io_error;
  std::filesystem::create_directories(target_root, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to prepare alias config target directory: " +
                             target_root.string() + " | " + io_error.message());
  }

  std::filesystem::copy(source_root, target_root,
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing,
                        io_error);
  if (io_error) {
    throw std::runtime_error("Failed to copy alias config directory: " +
                             source_root.string() + " -> " +
                             target_root.string() + " | " + io_error.message());
  }
}

}  // namespace tracer::core::application::pipeline::detail
