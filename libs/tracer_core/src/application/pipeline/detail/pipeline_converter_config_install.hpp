#ifndef APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_
#define APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_

#include <filesystem>
#include <string_view>

namespace tracer::core::application::pipeline::detail {

[[nodiscard]] auto ResolveConverterMainConfigPath(
    const std::filesystem::path& main_config_path) -> std::filesystem::path;

auto EnsureConverterConfigSourceExists(const std::filesystem::path& path,
                                       std::string_view label) -> void;

auto RemoveConverterUserConfigDirectory(
    const std::filesystem::path& target_root) -> void;

auto CopyConverterUserConfigDirectory(const std::filesystem::path& source_root,
                                      const std::filesystem::path& target_root)
    -> void;

}  // namespace tracer::core::application::pipeline::detail

#endif  // APPLICATION_PIPELINE_DETAIL_PIPELINE_CONVERTER_CONFIG_INSTALL_HPP_
