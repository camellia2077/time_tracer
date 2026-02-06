// config/config_loader.cpp
#include "config/config_loader.hpp"

#include <toml++/toml.h>

#include <iostream>
#include <stdexcept>

#include "config/internal/config_parser_utils.hpp"
#include "config/loader/report_config_loader.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
// [新增]
#include "config/loader/converter_config_loader.hpp"

namespace fs = std::filesystem;

namespace {
void LoadDetailedReports(AppConfig& config) {
  // --- Typst ---
  if (!config.reports.day_typ_config_path.empty()) {
    config.loaded_reports.typst.day = ReportConfigLoader::LoadDailyTypConfig(
        config.reports.day_typ_config_path);
  }
  if (!config.reports.month_typ_config_path.empty()) {
    config.loaded_reports.typst.month =
        ReportConfigLoader::LoadMonthlyTypConfig(
            config.reports.month_typ_config_path);
  }
  if (!config.reports.period_typ_config_path.empty()) {
    config.loaded_reports.typst.period =
        ReportConfigLoader::LoadPeriodTypConfig(
            config.reports.period_typ_config_path);
  }
  if (!config.reports.week_typ_config_path.empty()) {
    config.loaded_reports.typst.week = ReportConfigLoader::LoadWeeklyTypConfig(
        config.reports.week_typ_config_path);
  }
  if (!config.reports.year_typ_config_path.empty()) {
    config.loaded_reports.typst.year = ReportConfigLoader::LoadYearlyTypConfig(
        config.reports.year_typ_config_path);
  }

  // --- LaTeX ---
  if (!config.reports.day_tex_config_path.empty()) {
    config.loaded_reports.latex.day = ReportConfigLoader::LoadDailyTexConfig(
        config.reports.day_tex_config_path);
  }
  if (!config.reports.month_tex_config_path.empty()) {
    config.loaded_reports.latex.month =
        ReportConfigLoader::LoadMonthlyTexConfig(
            config.reports.month_tex_config_path);
  }
  if (!config.reports.period_tex_config_path.empty()) {
    config.loaded_reports.latex.period =
        ReportConfigLoader::LoadPeriodTexConfig(
            config.reports.period_tex_config_path);
  }
  if (!config.reports.week_tex_config_path.empty()) {
    config.loaded_reports.latex.week = ReportConfigLoader::LoadWeeklyTexConfig(
        config.reports.week_tex_config_path);
  }
  if (!config.reports.year_tex_config_path.empty()) {
    config.loaded_reports.latex.year = ReportConfigLoader::LoadYearlyTexConfig(
        config.reports.year_tex_config_path);
  }

  // --- Markdown ---
  if (!config.reports.day_md_config_path.empty()) {
    config.loaded_reports.markdown.day = ReportConfigLoader::LoadDailyMdConfig(
        config.reports.day_md_config_path);
  }
  if (!config.reports.month_md_config_path.empty()) {
    config.loaded_reports.markdown.month =
        ReportConfigLoader::LoadMonthlyMdConfig(
            config.reports.month_md_config_path);
  }
  if (!config.reports.period_md_config_path.empty()) {
    config.loaded_reports.markdown.period =
        ReportConfigLoader::LoadPeriodMdConfig(
            config.reports.period_md_config_path);
  }
  if (!config.reports.week_md_config_path.empty()) {
    config.loaded_reports.markdown.week =
        ReportConfigLoader::LoadWeeklyMdConfig(
            config.reports.week_md_config_path);
  }
  if (!config.reports.year_md_config_path.empty()) {
    config.loaded_reports.markdown.year =
        ReportConfigLoader::LoadYearlyMdConfig(
            config.reports.year_md_config_path);
  }
}
}  // namespace

ConfigLoader::ConfigLoader(const std::string& exe_path_str) {
  try {
    exe_path_ = fs::canonical(fs::path(exe_path_str)).parent_path();
  } catch (const fs::filesystem_error& e) {
    throw std::runtime_error("Failed to determine executable directory: " +
                             std::string(e.what()));
  }

  config_dir_path_ = exe_path_ / kConfigDirName;
  main_config_path_ = config_dir_path_ / kConfigFileName;
}

auto ConfigLoader::GetMainConfigPath() const -> std::string {
  return main_config_path_.string();
}

auto ConfigLoader::LoadConfiguration() -> AppConfig {
  if (!FileSystemHelper::Exists(main_config_path_)) {
    throw std::runtime_error("Configuration file not found: " +
                             main_config_path_.string());
  }

  toml::table tbl;
  try {
    tbl = toml::parse_file(main_config_path_.string());
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse config.toml: " +
                             std::string(err.description()));
  }

  AppConfig app_config;
  app_config.exe_dir_path = exe_path_;

  // 1. 解析基础路径和设置
  ConfigParserUtils::ParseSystemSettings(tbl, exe_path_, app_config);
  ConfigParserUtils::ParseCliDefaults(tbl, exe_path_, app_config);
  ConfigParserUtils::ParsePipelineSettings(tbl, config_dir_path_, app_config);
  ConfigParserUtils::ParseReportPaths(tbl, config_dir_path_, app_config);

  // 2. [新增] 加载 Converter 配置 (文件合并 + 解析逻辑现在在此处执行)
  try {
    if (!app_config.pipeline.interval_processor_config_path.empty()) {
      app_config.pipeline.loaded_converter_config =
          ConverterConfigLoader::LoadFromFile(
              app_config.pipeline.interval_processor_config_path);


      // 将 initial_top_parents 从 AppConfig 注入到 ConverterConfig 中
      // 确保 Core 模块不需要再手动处理这个合并
      for (const auto& [key, val] : app_config.pipeline.initial_top_parents) {
        app_config.pipeline.loaded_converter_config
            .initial_top_parents[key.string()] = val.string();
      }
    }
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to load converter configuration: " +
                             std::string(e.what()));
  }

  // 3. 加载报表配置
  try {
    LoadDetailedReports(app_config);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to load report configuration details: " +
                             std::string(e.what()));
  }

  return app_config;
}
