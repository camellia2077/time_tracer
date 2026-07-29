#ifndef TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_
#define TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_

#include <filesystem>
#include <string>
#include <string_view>

namespace tracer::core::application::config {

[[nodiscard]] auto RenderActivityHierarchyText(
    const std::filesystem::path& activity_hierarchy_toml_path,
    bool show_aliases) -> std::string;
[[nodiscard]] auto RenderActivityHierarchyText(std::string_view toml_content,
                                               bool show_aliases) -> std::string;

}  // namespace tracer::core::application::config

#endif  // TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_
