// api/cli/impl/commands/query/heatmap_config.hpp
#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace tracer_core::cli::impl::commands::query::heatmap {

struct HeatmapConfig {
  std::vector<double> positive_hours;
  std::string default_light_palette;
  std::string default_dark_palette;
  std::map<std::string, std::vector<std::string>> palettes;
};

auto ResolveHeatmapConfigPath(const std::filesystem::path &executable_path)
    -> std::filesystem::path;

auto LoadHeatmapConfig(const std::filesystem::path &config_path)
    -> HeatmapConfig;

auto ResolvePaletteColors(const HeatmapConfig &config,
                          std::string_view palette_name)
    -> std::vector<std::string>;

auto ListPaletteNames(const HeatmapConfig &config) -> std::vector<std::string>;

} // namespace tracer_core::cli::impl::commands::query::heatmap
