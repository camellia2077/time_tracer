#ifndef APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_
#define APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_

#include <filesystem>
#include <string_view>

namespace tracer::core::application::pipeline::detail {

struct ConverterConfigPathSet {
  std::filesystem::path main_config_path;
  std::filesystem::path alias_mapping_path;
  std::filesystem::path duration_rules_path;
};

[[nodiscard]] auto ResolveConverterConfigPathSet(
    const std::filesystem::path& main_config_path) -> ConverterConfigPathSet;

auto EnsureConverterConfigSourceExists(const std::filesystem::path& path,
                                       std::string_view label) -> void;

auto CopyConverterConfigFile(const std::filesystem::path& source_path,
                             const std::filesystem::path& target_path,
                             std::string_view label) -> void;

}  // namespace tracer::core::application::pipeline::detail

#endif  // APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_
