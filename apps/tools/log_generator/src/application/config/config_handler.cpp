// application/config/config_handler.cpp
#include "application/config/config_handler.hpp"

#include <iostream>

#include "common/ansi_colors.hpp"
#include "infrastructure/config/config.hpp"

namespace App {

ConfigHandler::ConfigHandler(FileSystem& file_system)
    : file_system_(file_system) {}

auto ConfigHandler::load(const Config& config,
                         const std::filesystem::path& exe_dir)
    -> std::optional<Core::AppContext> {
  Core::AppContext context;
  context.config = config;

  std::filesystem::path settings_path =
      exe_dir / "config" / "activities_config.toml";
  std::filesystem::path alias_mapping_path =
      exe_dir / "config" / "alias_mapping.toml";

  auto alias_mapping_content_opt = file_system_.read_file(alias_mapping_path);
  if (!alias_mapping_content_opt) {
    std::cerr << RED_COLOR << "Critical: Failed to read alias mapping config."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  auto settings_content_opt = file_system_.read_file(settings_path);
  if (!settings_content_opt) {
    std::cerr << RED_COLOR << "Critical: Failed to read settings config."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  auto toml_configs_opt =
      ConfigLoader::load_from_content(*settings_content_opt,
                                      *alias_mapping_content_opt);
  if (!toml_configs_opt) {
    std::cerr << RED_COLOR << "Config parse failed. Exiting." << RESET_COLOR
              << std::endl;
    return std::nullopt;
  }

  context.all_activities = toml_configs_opt->mapped_activities;
  if (context.all_activities.empty()) {
    std::cerr << RED_COLOR
              << "Critical: No mapped activities found in alias_mapping.toml."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  context.remarks = toml_configs_opt->remarks;
  context.activity_remarks = toml_configs_opt->activity_remarks;
  context.wake_keywords = toml_configs_opt->wake_keywords;
  if (toml_configs_opt->nosleep_probability.has_value()) {
    context.config.nosleep_probability = *toml_configs_opt->nosleep_probability;
  }

  return context;
}
}  // namespace App
