#include "infra/config/internal/config_detail_loader.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

import tracer.core.infrastructure.config.loader.report_config_loader;

using tracer::core::infrastructure::config::ReportConfigLoader;
namespace tracer::core::infrastructure::config::internal {

namespace {

auto LoadLocalizedMarkdownReports(const std::filesystem::path& markdown_dir,
                                  LoadedReportConfigs& reports) -> void {
  constexpr std::array<std::string_view, 5> kReportNames = {
      "day", "month", "period", "week", "year"};
  for (const auto& entry : std::filesystem::directory_iterator(markdown_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::string locale = entry.path().filename().string();
    MarkdownReportConfigs localized{};
    const auto path = [&entry](std::string_view name) {
      return entry.path() / (std::string(name) + ".toml");
    };
    const auto day_path = path(kReportNames[0]);
    const auto month_path = path(kReportNames[1]);
    const auto period_path = path(kReportNames[2]);
    const auto week_path = path(kReportNames[3]);
    const auto year_path = path(kReportNames[4]);
    if (!std::filesystem::exists(day_path) ||
        !std::filesystem::exists(month_path) ||
        !std::filesystem::exists(period_path) ||
        !std::filesystem::exists(week_path) ||
        !std::filesystem::exists(year_path)) {
      continue;
    }
    localized.day = ReportConfigLoader::LoadDailyMdConfig(day_path);
    localized.month = ReportConfigLoader::LoadMonthlyMdConfig(month_path);
    localized.period = ReportConfigLoader::LoadPeriodMdConfig(period_path);
    localized.week = ReportConfigLoader::LoadWeeklyMdConfig(week_path);
    localized.year = ReportConfigLoader::LoadYearlyMdConfig(year_path);
    reports.markdown_locales.emplace(locale, std::move(localized));
  }
}

}  // namespace

void LoadDetailedReports(AppConfig& config) {
  // Typst
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

  // LaTeX
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

  // Markdown
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
  if (!config.reports.day_md_config_path.empty()) {
    LoadLocalizedMarkdownReports(
        config.reports.day_md_config_path.parent_path(),
        config.loaded_reports);
  }
}

}  // namespace tracer::core::infrastructure::config::internal
