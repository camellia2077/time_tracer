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
  std::filesystem::path mapping_path =
      exe_dir / "config" / "mapping_config.toml";

  auto mapping_content_opt = file_system_.read_file(mapping_path);
  if (!mapping_content_opt) {
    std::cerr << RED_COLOR << "Critical: Failed to read mapping config."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  auto settings_content_opt = file_system_.read_file(settings_path);
  if (!settings_content_opt) {
    std::cerr << RED_COLOR << "Critical: Failed to read settings config."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  auto toml_configs_opt = ConfigLoader::load_from_content(
      *settings_content_opt, *mapping_content_opt);
  if (!toml_configs_opt) {
    std::cerr << RED_COLOR << "Config parse failed. Exiting." << RESET_COLOR
              << std::endl;
    return std::nullopt;
  }

  context.all_activities = toml_configs_opt->mapped_activities;
  if (context.all_activities.empty()) {
    std::cerr << RED_COLOR
              << "Critical: No mapped activities found in mapping_config.toml."
              << RESET_COLOR << std::endl;
    return std::nullopt;
  }

  context.remarks = toml_configs_opt->remarks;
  context.activity_remarks = toml_configs_opt->activity_remarks;
  context.wake_keywords = toml_configs_opt->wake_keywords;

  return context;
}
}  // namespace App
