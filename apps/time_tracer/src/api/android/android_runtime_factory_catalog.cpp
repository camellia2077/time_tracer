// api/android/android_runtime_factory_catalog.cpp
#include <stdexcept>

#include "api/android/android_runtime_factory_internal.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace infrastructure::bootstrap::android_runtime_detail {

auto BuildAndroidReportCatalog(
    const std::filesystem::path& output_root,
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> ReportCatalog {
  ReportCatalog catalog;
  catalog.plugin_dir_path = output_root / "plugins";

  catalog.loaded_reports.markdown.day =
      ReportConfigLoader::LoadDailyMdConfig(runtime_config_paths.markdown.day);
  catalog.loaded_reports.markdown.month =
      ReportConfigLoader::LoadMonthlyMdConfig(
          runtime_config_paths.markdown.month);
  catalog.loaded_reports.markdown.period =
      ReportConfigLoader::LoadPeriodMdConfig(
          runtime_config_paths.markdown.period);
  catalog.loaded_reports.markdown.week = ReportConfigLoader::LoadWeeklyMdConfig(
      runtime_config_paths.markdown.week);
  catalog.loaded_reports.markdown.year = ReportConfigLoader::LoadYearlyMdConfig(
      runtime_config_paths.markdown.year);

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

}  // namespace infrastructure::bootstrap::android_runtime_detail
