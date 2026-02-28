// infrastructure/config/models/app_config.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_APP_CONFIG_H_
#define INFRASTRUCTURE_CONFIG_MODELS_APP_CONFIG_H_

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/types/date_check_mode.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace fs = std::filesystem;

struct PipelineConfig {
  fs::path interval_processor_config_path;
  std::unordered_map<fs::path, fs::path> initial_top_parents;
};

struct ReportConfigPaths {
  // Typst Report Configs
  fs::path day_typ_config_path;
  fs::path month_typ_config_path;
  fs::path period_typ_config_path;
  fs::path week_typ_config_path;
  fs::path year_typ_config_path;

  // LaTeX Report Configs
  fs::path day_tex_config_path;
  fs::path month_tex_config_path;
  fs::path period_tex_config_path;
  fs::path week_tex_config_path;
  fs::path year_tex_config_path;

  // Markdown Report Configs
  fs::path day_md_config_path;
  fs::path month_md_config_path;
  fs::path period_md_config_path;
  fs::path week_md_config_path;
  fs::path year_md_config_path;
};

struct GlobalDefaults {
  std::optional<fs::path> kDbPath;
  std::optional<fs::path> output_root;
  std::optional<std::string> default_format;
};

struct CommandDefaults {
  std::optional<std::string> export_format;
  std::optional<std::string> query_format;
  std::optional<DateCheckMode> convert_date_check_mode;
  std::optional<bool> convert_save_processed_output;
  std::optional<bool> convert_validate_logic;
  std::optional<bool> convert_validate_structure;
  std::optional<DateCheckMode> ingest_date_check_mode;
  std::optional<bool> ingest_save_processed_output;
  std::optional<DateCheckMode> validate_logic_date_check_mode;
};

struct AppConfig {
  fs::path exe_dir_path;
  fs::path error_log_path;
  std::optional<fs::path> kExportPath;

  bool default_save_processed_output = false;
  DateCheckMode default_date_check_mode = DateCheckMode::kNone;

  PipelineConfig pipeline;
  ReportConfigPaths reports;
  GlobalDefaults defaults;
  CommandDefaults command_defaults;

  LoadedReportConfigs loaded_reports;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_APP_CONFIG_H_
