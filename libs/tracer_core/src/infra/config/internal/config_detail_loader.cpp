#include "infra/config/internal/config_detail_loader.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

import tracer.core.infrastructure.config.loader.insights_config_loader;

using tracer::core::infrastructure::config::InsightsConfigLoader;
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

auto LoadLocalizedMarkdownInsights(const std::filesystem::path& markdown_dir,
                                  LoadedInsightsConfigs& insights) -> void {
  constexpr std::array<std::string_view, 5> kInsightsNames = {
      "day", "month", "period", "week", "year"};
  for (const auto& entry : std::filesystem::directory_iterator(markdown_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::string kLocale = entry.path().filename().string();
    MarkdownInsightsConfigs localized{};
    const auto kPath = [&entry](std::string_view name) {
      return entry.path() / (std::string(name) + ".toml");
    };
    const auto kDayPath = kPath(kInsightsNames[0]);
    const auto kMonthPath = kPath(kInsightsNames[1]);
    const auto kPeriodPath = kPath(kInsightsNames[2]);
    const auto kWeekPath = kPath(kInsightsNames[3]);
    const auto kYearPath = kPath(kInsightsNames[4]);
    if (!std::filesystem::exists(kDayPath) ||
        !std::filesystem::exists(kMonthPath) ||
        !std::filesystem::exists(kPeriodPath) ||
        !std::filesystem::exists(kWeekPath) ||
        !std::filesystem::exists(kYearPath)) {
      continue;
    }
    localized.day = InsightsConfigLoader::LoadDailyMdConfig(kDayPath);
    localized.month = InsightsConfigLoader::LoadMonthlyMdConfig(kMonthPath);
    localized.period = InsightsConfigLoader::LoadPeriodMdConfig(kPeriodPath);
    localized.week = InsightsConfigLoader::LoadWeeklyMdConfig(kWeekPath);
    localized.year = InsightsConfigLoader::LoadYearlyMdConfig(kYearPath);
    insights.markdown_locales.emplace(kLocale, std::move(localized));
  }
}

}  // namespace

void LoadDetailedInsights(AppConfig& config) {
  // Typst
  if (!config.insights.day_typ_config_path.empty()) {
    config.loaded_insights.typst.day = InsightsConfigLoader::LoadDailyTypConfig(
        config.insights.day_typ_config_path);
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
    config.loaded_insights.typst.week = InsightsConfigLoader::LoadWeeklyTypConfig(
        config.insights.week_typ_config_path);
  }
  if (!config.insights.year_typ_config_path.empty()) {
    config.loaded_insights.typst.year = InsightsConfigLoader::LoadYearlyTypConfig(
        config.insights.year_typ_config_path);
  }

  // LaTeX
  if (!config.insights.day_tex_config_path.empty()) {
    config.loaded_insights.latex.day = InsightsConfigLoader::LoadDailyTexConfig(
        config.insights.day_tex_config_path);
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
    config.loaded_insights.latex.week = InsightsConfigLoader::LoadWeeklyTexConfig(
        config.insights.week_tex_config_path);
  }
  if (!config.insights.year_tex_config_path.empty()) {
    config.loaded_insights.latex.year = InsightsConfigLoader::LoadYearlyTexConfig(
        config.insights.year_tex_config_path);
  }

  // Markdown
  if (!config.insights.day_md_config_path.empty()) {
    config.loaded_insights.markdown.day = InsightsConfigLoader::LoadDailyMdConfig(
        config.insights.day_md_config_path);
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
  if (!config.insights.day_md_config_path.empty()) {
    LoadLocalizedMarkdownInsights(
        ResolveMarkdownLocaleRoot(config.insights.day_md_config_path),
        config.loaded_insights);
  }
}

}  // namespace tracer::core::infrastructure::config::internal
