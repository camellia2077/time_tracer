// infrastructure/config/models/report_catalog.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_
#define INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_

#include <filesystem>

#include "infrastructure/config/models/report_config_models.hpp"

struct LoadedReportConfigs {
  struct {
    DailyTexConfig day;
    MonthlyTexConfig month;
    PeriodTexConfig period;
    WeeklyTexConfig week;
    YearlyTexConfig year;
  } latex;

  struct {
    DailyTypConfig day;
    MonthlyTypConfig month;
    PeriodTypConfig period;
    WeeklyTypConfig week;
    YearlyTypConfig year;
  } typst;

  struct {
    DailyMdConfig day;
    MonthlyMdConfig month;
    PeriodMdConfig period;
    WeeklyMdConfig week;
    YearlyMdConfig year;
  } markdown;
};

struct ReportCatalog {
  std::filesystem::path plugin_dir_path;
  LoadedReportConfigs loaded_reports;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_
