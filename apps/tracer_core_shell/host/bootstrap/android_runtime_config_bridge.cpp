#include "host/bootstrap/android_runtime_config_bridge.hpp"

import tracer.core.infrastructure.config.internal.android_bundle_config_paths;
import tracer.core.infrastructure.config.loader.report_config_loader;

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace tracer_core::shell::config_bridge {

namespace modconfig = tracer::core::infrastructure::modconfig;
namespace modconfig_internal = tracer::core::infrastructure::modconfig::internal;
using ReportConfigLoader = modconfig::ReportConfigLoader;
using AndroidRuntimeConfigPaths =
    ::infrastructure::bootstrap::android_runtime_detail::AndroidRuntimeConfigPaths;
using AndroidReportConfigPathSet =
    ::infrastructure::bootstrap::android_runtime_detail::AndroidReportConfigPathSet;

namespace {

namespace fs = std::filesystem;
namespace infra_config_internal = modconfig_internal;

#include "host/bootstrap/internal/android_runtime_factory_resolver_namespace.inc"

}  // namespace

auto ResolveAndroidRuntimeConfigPathsBridge(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths {
  const fs::path requested_converter_config_path = RequireNonEmptyPath(
      requested_converter_config_toml_path, "converter_config_toml_path");
  const fs::path config_root =
      ResolveReportConfigRoot(requested_converter_config_path);

  const std::optional<modconfig_internal::AndroidBundleConfigPaths> bundle_paths =
      modconfig_internal::TryResolveAndroidBundleConfigPaths(config_root);
  if (bundle_paths.has_value()) {
#if TT_REPORT_ENABLE_LATEX
    const std::optional<AndroidReportConfigPathSet> latex_paths =
        bundle_paths->latex.has_value()
            ? std::optional<AndroidReportConfigPathSet>(
                  ToAndroidReportConfigPathSet(*bundle_paths->latex))
            : std::nullopt;
#else
    if (bundle_paths->latex.has_value()) {
      throw std::runtime_error(
          "Android runtime bundle contains LaTeX report paths, but this core "
          "build disables LaTeX (TT_REPORT_ENABLE_LATEX=OFF).");
    }
    const std::optional<AndroidReportConfigPathSet> latex_paths = std::nullopt;
#endif

#if TT_REPORT_ENABLE_TYPST
    const std::optional<AndroidReportConfigPathSet> typst_paths =
        bundle_paths->typst.has_value()
            ? std::optional<AndroidReportConfigPathSet>(
                  ToAndroidReportConfigPathSet(*bundle_paths->typst))
            : std::nullopt;
#else
    if (bundle_paths->typst.has_value()) {
      throw std::runtime_error(
          "Android runtime bundle contains Typst report paths, but this core "
          "build disables Typst (TT_REPORT_ENABLE_TYPST=OFF).");
    }
    const std::optional<AndroidReportConfigPathSet> typst_paths = std::nullopt;
#endif

    return {
        .converter_config_toml_path = ResolveConverterConfigTomlPath(
            bundle_paths->converter_config_toml_path),
        .markdown = ToAndroidReportConfigPathSet(bundle_paths->markdown),
        .latex = latex_paths,
        .typst = typst_paths,
        .formatter_policy = {.enable_markdown = true,
                             .enable_latex = latex_paths.has_value(),
                             .enable_typst = typst_paths.has_value()},
    };
  }

  return {
      .converter_config_toml_path =
          ResolveConverterConfigTomlPath(requested_converter_config_path),
      .markdown = BuildLegacyMarkdownConfigPaths(config_root),
      .latex = std::nullopt,
      .typst = std::nullopt,
      .formatter_policy = ::infrastructure::reports::
          AndroidStaticReportFormatterPolicy::MarkdownOnly(),
  };
}

auto BuildAndroidReportCatalogBridge(
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> ReportCatalog {
  ReportCatalog catalog;

  catalog.loaded_reports.markdown.day =
      ReportConfigLoader::LoadDailyMdConfig(runtime_config_paths.markdown.day);
  catalog.loaded_reports.markdown.month =
      ReportConfigLoader::LoadMonthlyMdConfig(
          runtime_config_paths.markdown.month);
  catalog.loaded_reports.markdown.period =
      ReportConfigLoader::LoadPeriodMdConfig(
          runtime_config_paths.markdown.period);
  catalog.loaded_reports.markdown.week =
      ReportConfigLoader::LoadWeeklyMdConfig(runtime_config_paths.markdown.week);
  catalog.loaded_reports.markdown.year =
      ReportConfigLoader::LoadYearlyMdConfig(runtime_config_paths.markdown.year);

#if TT_REPORT_ENABLE_LATEX
  if (runtime_config_paths.latex.has_value()) {
    const auto& latex = *runtime_config_paths.latex;
    catalog.loaded_reports.latex.day =
        ReportConfigLoader::LoadDailyTexConfig(latex.day);
    catalog.loaded_reports.latex.month =
        ReportConfigLoader::LoadMonthlyTexConfig(latex.month);
    catalog.loaded_reports.latex.period =
        ReportConfigLoader::LoadPeriodTexConfig(latex.period);
    catalog.loaded_reports.latex.week =
        ReportConfigLoader::LoadWeeklyTexConfig(latex.week);
    catalog.loaded_reports.latex.year =
        ReportConfigLoader::LoadYearlyTexConfig(latex.year);
  }
#else
  if (runtime_config_paths.latex.has_value()) {
    throw std::runtime_error(
        "Android runtime LaTeX config paths are present, but LaTeX support "
        "is disabled at compile time (TT_REPORT_ENABLE_LATEX=OFF).");
  }
#endif

#if TT_REPORT_ENABLE_TYPST
  if (runtime_config_paths.typst.has_value()) {
    const auto& typst = *runtime_config_paths.typst;
    catalog.loaded_reports.typst.day =
        ReportConfigLoader::LoadDailyTypConfig(typst.day);
    catalog.loaded_reports.typst.month =
        ReportConfigLoader::LoadMonthlyTypConfig(typst.month);
    catalog.loaded_reports.typst.period =
        ReportConfigLoader::LoadPeriodTypConfig(typst.period);
    catalog.loaded_reports.typst.week =
        ReportConfigLoader::LoadWeeklyTypConfig(typst.week);
    catalog.loaded_reports.typst.year =
        ReportConfigLoader::LoadYearlyTypConfig(typst.year);
  }
#else
  if (runtime_config_paths.typst.has_value()) {
    throw std::runtime_error(
        "Android runtime Typst config paths are present, but Typst support "
        "is disabled at compile time (TT_REPORT_ENABLE_TYPST=OFF).");
  }
#endif

  return catalog;
}

}  // namespace tracer_core::shell::config_bridge
