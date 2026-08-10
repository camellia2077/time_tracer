#include "host/bootstrap/android_runtime_config_bridge.hpp"

import tracer.core.infrastructure.config.internal.android_bundle_config_paths;
import tracer.core.infrastructure.config.loader.insights_config_loader;

#include <filesystem>
#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifndef TT_INSIGHTS_ENABLE_LATEX
#define TT_INSIGHTS_ENABLE_LATEX 1
#endif

#ifndef TT_INSIGHTS_ENABLE_TYPST
#define TT_INSIGHTS_ENABLE_TYPST 1
#endif

namespace tracer_core::shell::config_bridge {

namespace modconfig = tracer::core::infrastructure::modconfig;
namespace modconfig_internal =
    tracer::core::infrastructure::modconfig::internal;
using InsightsConfigLoader = modconfig::InsightsConfigLoader;
using AndroidRuntimeConfigPaths = ::infrastructure::bootstrap::
    android_runtime_detail::AndroidRuntimeConfigPaths;
using AndroidInsightsConfigPathSet = ::infrastructure::bootstrap::
    android_runtime_detail::AndroidInsightsConfigPathSet;

namespace {

namespace fs = std::filesystem;
namespace infra_config_internal = modconfig_internal;

auto LoadLocalizedMarkdownInsights(const fs::path& markdown_dir,
                                  InsightsCatalog& catalog) -> void {
  constexpr std::array<std::string_view, 5> kInsightsNames = {
      "day", "month", "period", "week", "year"};
  if (!fs::exists(markdown_dir) || !fs::is_directory(markdown_dir)) {
    return;
  }

  for (const auto& entry : fs::directory_iterator(markdown_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    const auto path = [&entry](std::string_view name) {
      return entry.path() / (std::string(name) + ".toml");
    };
    const auto day_path = path(kInsightsNames[0]);
    const auto month_path = path(kInsightsNames[1]);
    const auto period_path = path(kInsightsNames[2]);
    const auto week_path = path(kInsightsNames[3]);
    const auto year_path = path(kInsightsNames[4]);
    if (!fs::exists(day_path) || !fs::exists(month_path) ||
        !fs::exists(period_path) || !fs::exists(week_path) ||
        !fs::exists(year_path)) {
      continue;
    }

    MarkdownInsightsConfigs localized;
    localized.day = InsightsConfigLoader::LoadDailyMdConfig(day_path);
    localized.month = InsightsConfigLoader::LoadMonthlyMdConfig(month_path);
    localized.period = InsightsConfigLoader::LoadPeriodMdConfig(period_path);
    localized.week = InsightsConfigLoader::LoadWeeklyMdConfig(week_path);
    localized.year = InsightsConfigLoader::LoadYearlyMdConfig(year_path);
    catalog.loaded_insights.markdown_locales.emplace(
        entry.path().filename().string(), std::move(localized));
  }
}

auto ResolveMarkdownLocaleRoot(const fs::path& day_config_path) -> fs::path {
  const fs::path parent = day_config_path.parent_path();
  const fs::path candidate = parent.parent_path();
  if (!fs::exists(candidate) || !fs::is_directory(candidate)) {
    return parent;
  }
  for (const auto& entry : fs::directory_iterator(candidate)) {
    if (!entry.is_directory()) {
      continue;
    }
    if (fs::exists(entry.path() / "day.toml") &&
        fs::exists(entry.path() / "month.toml") &&
        fs::exists(entry.path() / "period.toml") &&
        fs::exists(entry.path() / "week.toml") &&
        fs::exists(entry.path() / "year.toml")) {
      return candidate;
    }
  }
  return parent;
}

#include "host/bootstrap/internal/android_runtime_factory_resolver_namespace.inc"

}  // namespace

auto ResolveAndroidRuntimeConfigPathsBridge(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths {
  const fs::path requested_converter_config_path = RequireNonEmptyPath(
      requested_converter_config_toml_path, "converter_config_toml_path");
  const fs::path config_root =
      ResolveInsightsConfigRoot(requested_converter_config_path);

  const std::optional<modconfig_internal::AndroidBundleConfigPaths>
      bundle_paths =
          modconfig_internal::TryResolveAndroidBundleConfigPaths(config_root);
  if (bundle_paths.has_value()) {
#if TT_INSIGHTS_ENABLE_LATEX
    const std::optional<AndroidInsightsConfigPathSet> latex_paths =
        bundle_paths->latex.has_value()
            ? std::optional<AndroidInsightsConfigPathSet>(
                  ToAndroidInsightsConfigPathSet(*bundle_paths->latex))
            : std::nullopt;
#else
    if (bundle_paths->latex.has_value()) {
      throw std::runtime_error(
          "Android runtime bundle contains LaTeX insights paths, but this core "
          "build disables LaTeX (TT_INSIGHTS_ENABLE_LATEX=OFF).");
    }
    const std::optional<AndroidInsightsConfigPathSet> latex_paths = std::nullopt;
#endif

#if TT_INSIGHTS_ENABLE_TYPST
    const std::optional<AndroidInsightsConfigPathSet> typst_paths =
        bundle_paths->typst.has_value()
            ? std::optional<AndroidInsightsConfigPathSet>(
                  ToAndroidInsightsConfigPathSet(*bundle_paths->typst))
            : std::nullopt;
#else
    if (bundle_paths->typst.has_value()) {
      throw std::runtime_error(
          "Android runtime bundle contains Typst insights paths, but this core "
          "build disables Typst (TT_INSIGHTS_ENABLE_TYPST=OFF).");
    }
    const std::optional<AndroidInsightsConfigPathSet> typst_paths = std::nullopt;
#endif

    return {
        .converter_config_toml_path = ResolveConverterConfigTomlPath(
            bundle_paths->converter_config_toml_path),
        .markdown = ToAndroidInsightsConfigPathSet(bundle_paths->markdown),
        .latex = latex_paths,
        .typst = typst_paths,
        .formatter_policy = {.enable_markdown = true,
                             .enable_latex = latex_paths.has_value(),
                             .enable_typst = typst_paths.has_value()},
    };
  }

  throw std::runtime_error("Android runtime config bundle not found under: " +
                           config_root.string());
}

auto BuildAndroidInsightsCatalogBridge(
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> InsightsCatalog {
  InsightsCatalog catalog;

  const fs::path insights_status_config_path =
      runtime_config_paths.converter_config_toml_path.parent_path() /
      "insights.toml";
  if (fs::exists(insights_status_config_path)) {
    catalog.daily_statuses =
        InsightsConfigLoader::LoadDailyStatusConfig(insights_status_config_path);
  }

  catalog.loaded_insights.markdown.day =
      InsightsConfigLoader::LoadDailyMdConfig(runtime_config_paths.markdown.day);
  catalog.loaded_insights.markdown.month =
      InsightsConfigLoader::LoadMonthlyMdConfig(
          runtime_config_paths.markdown.month);
  catalog.loaded_insights.markdown.period =
      InsightsConfigLoader::LoadPeriodMdConfig(
          runtime_config_paths.markdown.period);
  catalog.loaded_insights.markdown.week = InsightsConfigLoader::LoadWeeklyMdConfig(
      runtime_config_paths.markdown.week);
  catalog.loaded_insights.markdown.year = InsightsConfigLoader::LoadYearlyMdConfig(
      runtime_config_paths.markdown.year);
  LoadLocalizedMarkdownInsights(
      ResolveMarkdownLocaleRoot(runtime_config_paths.markdown.day), catalog);

#if TT_INSIGHTS_ENABLE_LATEX
  if (runtime_config_paths.latex.has_value()) {
    const auto& latex = *runtime_config_paths.latex;
    catalog.loaded_insights.latex.day =
        InsightsConfigLoader::LoadDailyTexConfig(latex.day);
    catalog.loaded_insights.latex.month =
        InsightsConfigLoader::LoadMonthlyTexConfig(latex.month);
    catalog.loaded_insights.latex.period =
        InsightsConfigLoader::LoadPeriodTexConfig(latex.period);
    catalog.loaded_insights.latex.week =
        InsightsConfigLoader::LoadWeeklyTexConfig(latex.week);
    catalog.loaded_insights.latex.year =
        InsightsConfigLoader::LoadYearlyTexConfig(latex.year);
  }
#else
  if (runtime_config_paths.latex.has_value()) {
    throw std::runtime_error(
        "Android runtime LaTeX config paths are present, but LaTeX support "
        "is disabled at compile time (TT_INSIGHTS_ENABLE_LATEX=OFF).");
  }
#endif

#if TT_INSIGHTS_ENABLE_TYPST
  if (runtime_config_paths.typst.has_value()) {
    const auto& typst = *runtime_config_paths.typst;
    catalog.loaded_insights.typst.day =
        InsightsConfigLoader::LoadDailyTypConfig(typst.day);
    catalog.loaded_insights.typst.month =
        InsightsConfigLoader::LoadMonthlyTypConfig(typst.month);
    catalog.loaded_insights.typst.period =
        InsightsConfigLoader::LoadPeriodTypConfig(typst.period);
    catalog.loaded_insights.typst.week =
        InsightsConfigLoader::LoadWeeklyTypConfig(typst.week);
    catalog.loaded_insights.typst.year =
        InsightsConfigLoader::LoadYearlyTypConfig(typst.year);
  }
#else
  if (runtime_config_paths.typst.has_value()) {
    throw std::runtime_error(
        "Android runtime Typst config paths are present, but Typst support "
        "is disabled at compile time (TT_INSIGHTS_ENABLE_TYPST=OFF).");
  }
#endif

  return catalog;
}

}  // namespace tracer_core::shell::config_bridge
