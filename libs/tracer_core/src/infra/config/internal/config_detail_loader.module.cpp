module;

#include "infra/config/models/app_config.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

module tracer.core.infrastructure.config.internal.config_detail_loader;

import tracer.core.infrastructure.config.loader.insights_config_loader;

using tracer::core::infrastructure::config::InsightsConfigLoader;

namespace tracer::core::infrastructure::config::internal {

namespace {

auto ResolveMarkdownLocaleRoot(const std::filesystem::path& day_config_path)
    -> std::filesystem::path {
  const auto parent = day_config_path.parent_path();
  const auto candidate = parent.parent_path();
  if (!std::filesystem::exists(candidate) ||
      !std::filesystem::is_directory(candidate)) {
    return parent;
  }
  for (const auto& entry : std::filesystem::directory_iterator(candidate)) {
    if (!entry.is_directory()) {
      continue;
    }
    if (std::filesystem::exists(entry.path() / "day.toml") &&
        std::filesystem::exists(entry.path() / "month.toml") &&
        std::filesystem::exists(entry.path() / "period.toml") &&
        std::filesystem::exists(entry.path() / "week.toml") &&
        std::filesystem::exists(entry.path() / "year.toml")) {
      return candidate;
    }
  }
  return parent;
}

auto LoadLocalizedMarkdownInsights(const std::filesystem::path& markdown_dir,
                                   LoadedInsightsConfigs& insights) -> void {
  constexpr std::array<std::string_view, 5> kInsightsNames = {
      "day", "month", "period", "week", "year"};
  for (const auto& entry : std::filesystem::directory_iterator(markdown_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::string locale = entry.path().filename().string();
    MarkdownInsightsConfigs localized{};
    const auto path = [&entry](std::string_view name) {
      return entry.path() / (std::string(name) + ".toml");
    };
    const auto day_path = path(kInsightsNames[0]);
    const auto month_path = path(kInsightsNames[1]);
    const auto period_path = path(kInsightsNames[2]);
    const auto week_path = path(kInsightsNames[3]);
    const auto year_path = path(kInsightsNames[4]);
    if (!std::filesystem::exists(day_path) ||
        !std::filesystem::exists(month_path) ||
        !std::filesystem::exists(period_path) ||
        !std::filesystem::exists(week_path) ||
        !std::filesystem::exists(year_path)) {
      continue;
    }
    localized.daily = InsightsConfigLoader::LoadDailyMdConfig(day_path);
    localized.month = InsightsConfigLoader::LoadMonthlyMdConfig(month_path);
    localized.period = InsightsConfigLoader::LoadPeriodMdConfig(period_path);
    localized.week = InsightsConfigLoader::LoadWeeklyMdConfig(week_path);
    localized.year = InsightsConfigLoader::LoadYearlyMdConfig(year_path);
    insights.markdown_locales.emplace(locale, std::move(localized));
  }
}

}  // namespace

void LoadDetailedInsights(AppConfig& config) {
  if (!config.insights.daily_typ_config_path.empty()) {
    config.loaded_insights.typst.daily =
        InsightsConfigLoader::LoadDailyTypConfig(
            config.insights.daily_typ_config_path);
  }
  if (!config.insights.month_typ_config_path.empty()) {
    config.loaded_insights.typst.month =
        InsightsConfigLoader::LoadMonthlyTypConfig(
            config.insights.month_typ_config_path);
  }
  if (!config.insights.period_typ_config_path.empty()) {
    config.loaded_insights.typst.period =
        InsightsConfigLoader::LoadPeriodTypConfig(
            config.insights.period_typ_config_path);
  }
  if (!config.insights.week_typ_config_path.empty()) {
    config.loaded_insights.typst.week =
        InsightsConfigLoader::LoadWeeklyTypConfig(
            config.insights.week_typ_config_path);
  }
  if (!config.insights.year_typ_config_path.empty()) {
    config.loaded_insights.typst.year =
        InsightsConfigLoader::LoadYearlyTypConfig(
            config.insights.year_typ_config_path);
  }

  if (!config.insights.daily_tex_config_path.empty()) {
    config.loaded_insights.latex.daily =
        InsightsConfigLoader::LoadDailyTexConfig(
            config.insights.daily_tex_config_path);
  }
  if (!config.insights.month_tex_config_path.empty()) {
    config.loaded_insights.latex.month =
        InsightsConfigLoader::LoadMonthlyTexConfig(
            config.insights.month_tex_config_path);
  }
  if (!config.insights.period_tex_config_path.empty()) {
    config.loaded_insights.latex.period =
        InsightsConfigLoader::LoadPeriodTexConfig(
            config.insights.period_tex_config_path);
  }
  if (!config.insights.week_tex_config_path.empty()) {
    config.loaded_insights.latex.week =
        InsightsConfigLoader::LoadWeeklyTexConfig(
            config.insights.week_tex_config_path);
  }
  if (!config.insights.year_tex_config_path.empty()) {
    config.loaded_insights.latex.year =
        InsightsConfigLoader::LoadYearlyTexConfig(
            config.insights.year_tex_config_path);
  }

  if (!config.insights.daily_md_config_path.empty()) {
    config.loaded_insights.markdown.daily =
        InsightsConfigLoader::LoadDailyMdConfig(
            config.insights.daily_md_config_path);
  }
  if (!config.insights.month_md_config_path.empty()) {
    config.loaded_insights.markdown.month =
        InsightsConfigLoader::LoadMonthlyMdConfig(
            config.insights.month_md_config_path);
  }
  if (!config.insights.period_md_config_path.empty()) {
    config.loaded_insights.markdown.period =
        InsightsConfigLoader::LoadPeriodMdConfig(
            config.insights.period_md_config_path);
  }
  if (!config.insights.week_md_config_path.empty()) {
    config.loaded_insights.markdown.week =
        InsightsConfigLoader::LoadWeeklyMdConfig(
            config.insights.week_md_config_path);
  }
  if (!config.insights.year_md_config_path.empty()) {
    config.loaded_insights.markdown.year =
        InsightsConfigLoader::LoadYearlyMdConfig(
            config.insights.year_md_config_path);
  }
  if (!config.insights.daily_md_config_path.empty()) {
    LoadLocalizedMarkdownInsights(
        ResolveMarkdownLocaleRoot(config.insights.daily_md_config_path),
        config.loaded_insights);
  }
}

}  // namespace tracer::core::infrastructure::config::internal
