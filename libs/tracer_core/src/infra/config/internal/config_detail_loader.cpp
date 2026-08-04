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

auto ResolveMarkdownLocaleRoot(const std::filesystem::path& day_config_path)
    -> std::filesystem::path {
  const auto kParent = day_config_path.parent_path();
  const auto kCandidate = kParent.parent_path();
  if (!std::filesystem::exists(kCandidate) ||
      !std::filesystem::is_directory(kCandidate)) {
    return kParent;
  }
  for (const auto& entry : std::filesystem::directory_iterator(kCandidate)) {
    if (!entry.is_directory()) {
      continue;
    }
    if (std::filesystem::exists(entry.path() / "day.toml") &&
        std::filesystem::exists(entry.path() / "month.toml") &&
        std::filesystem::exists(entry.path() / "period.toml") &&
        std::filesystem::exists(entry.path() / "week.toml") &&
        std::filesystem::exists(entry.path() / "year.toml")) {
      return kCandidate;
    }
  }
  return kParent;
}

auto LoadLocalizedMarkdownReports(const std::filesystem::path& markdown_dir,
                                  LoadedReportConfigs& reports) -> void {
  constexpr std::array<std::string_view, 5> kReportNames = {
      "day", "month", "period", "week", "year"};
  for (const auto& entry : std::filesystem::directory_iterator(markdown_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::string kLocale = entry.path().filename().string();
    MarkdownReportConfigs localized{};
    const auto kPath = [&entry](std::string_view name) {
      return entry.path() / (std::string(name) + ".toml");
    };
    const auto kDayPath = kPath(kReportNames[0]);
    const auto kMonthPath = kPath(kReportNames[1]);
    const auto kPeriodPath = kPath(kReportNames[2]);
    const auto kWeekPath = kPath(kReportNames[3]);
    const auto kYearPath = kPath(kReportNames[4]);
    if (!std::filesystem::exists(kDayPath) ||
        !std::filesystem::exists(kMonthPath) ||
        !std::filesystem::exists(kPeriodPath) ||
        !std::filesystem::exists(kWeekPath) ||
        !std::filesystem::exists(kYearPath)) {
      continue;
    }
    localized.day = ReportConfigLoader::LoadDailyMdConfig(kDayPath);
    localized.month = ReportConfigLoader::LoadMonthlyMdConfig(kMonthPath);
    localized.period = ReportConfigLoader::LoadPeriodMdConfig(kPeriodPath);
    localized.week = ReportConfigLoader::LoadWeeklyMdConfig(kWeekPath);
    localized.year = ReportConfigLoader::LoadYearlyMdConfig(kYearPath);
    reports.markdown_locales.emplace(kLocale, std::move(localized));
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
        ResolveMarkdownLocaleRoot(config.reports.day_md_config_path),
        config.loaded_reports);
  }
}

}  // namespace tracer::core::infrastructure::config::internal
