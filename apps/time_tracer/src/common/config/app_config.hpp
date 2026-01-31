// common/config/app_config.hpp
#ifndef COMMON_CONFIG_APP_CONFIG_H_
#define COMMON_CONFIG_APP_CONFIG_H_

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/config/report_config_models.hpp"
#include "domain/types/date_check_mode.hpp"
// [新增] 引入 Converter 配置模型
#include "common/config/models/converter_config_models.hpp"

namespace fs = std::filesystem;

// [新增] 用于存储加载后的所有报表配置
struct LoadedReportConfigs {
  struct {
    DailyTexConfig day;
    MonthlyTexConfig month;
    PeriodTexConfig period;
  } latex;

  struct {
    DailyTypConfig day;
    MonthlyTypConfig month;
    PeriodTypConfig period;
  } typst;

  struct {
    DailyMdConfig day;
    MonthlyMdConfig month;
    PeriodMdConfig period;
  } markdown;
};

struct PipelineConfig {
  fs::path interval_processor_config_path;
  std::unordered_map<fs::path, fs::path> initial_top_parents;

  // [新增] 加载完成的 Converter 配置
  // Core 层将直接使用此对象初始化 Converter，而无需 Core 自己去解析 TOML
  ConverterConfig loaded_converter_config;
};

struct ReportConfigPaths {
  // Typst Report Configs
  fs::path day_typ_config_path;
  fs::path month_typ_config_path;
  fs::path period_typ_config_path;

  // LaTeX Report Configs
  fs::path day_tex_config_path;
  fs::path month_tex_config_path;
  fs::path period_tex_config_path;

  // Markdown Report Configs
  fs::path day_md_config_path;
  fs::path month_md_config_path;
  fs::path period_md_config_path;
};

struct AppConfig {
  fs::path exe_dir_path;
  fs::path error_log_path;
  std::optional<fs::path> export_path;

  bool default_save_processed_output = false;
  DateCheckMode default_date_check_mode = DateCheckMode::None;

  PipelineConfig pipeline;
  ReportConfigPaths reports;

  LoadedReportConfigs loaded_reports;
};

#endif  // COMMON_CONFIG_APP_CONFIG_H_
