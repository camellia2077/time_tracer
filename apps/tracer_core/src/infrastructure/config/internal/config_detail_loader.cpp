// infrastructure/config/internal/config_detail_loader.cpp
#include "infrastructure/config/internal/config_detail_loader.hpp"

#include "infrastructure/config/loader/report_config_loader.hpp"

namespace ConfigDetailLoader {

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
}

}  // namespace ConfigDetailLoader
