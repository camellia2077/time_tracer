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

}  // namespace tracer::core::application::pipeline::detail
