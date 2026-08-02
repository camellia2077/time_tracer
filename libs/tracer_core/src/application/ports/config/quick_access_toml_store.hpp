#ifndef TRACER_CORE_APPLICATION_PORTS_CONFIG_QUICK_ACCESS_TOML_STORE_HPP_
#define TRACER_CORE_APPLICATION_PORTS_CONFIG_QUICK_ACCESS_TOML_STORE_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace tracer::core::application::config {

struct QuickAccessConfig {
  std::vector<std::string> aliases;
};

// Parses Quick Access TOML content. File existence and file I/O belong to the
// platform layer, not Core.
[[nodiscard]] auto ParseQuickAccessToml(std::string_view content)
    -> QuickAccessConfig;

// Validates and renders Quick Access aliases as canonical TOML. File creation
// and file I/O belong to the platform layer, not Core.
[[nodiscard]] auto RenderQuickAccessToml(const QuickAccessConfig& config)
    -> std::string;

}  // namespace tracer::core::application::config

#endif  // TRACER_CORE_APPLICATION_PORTS_CONFIG_QUICK_ACCESS_TOML_STORE_HPP_
