// infra/config/models/report_catalog.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_
#define INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_

#include <string>
#include <unordered_map>

#include "infra/config/models/report_config_models.hpp"

struct MarkdownReportConfigs {
  DailyMdConfig day;
  MonthlyMdConfig month;
  PeriodMdConfig period;
  WeeklyMdConfig week;
  YearlyMdConfig year;
};

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

  MarkdownReportConfigs markdown;
  std::unordered_map<std::string, MarkdownReportConfigs> markdown_locales;
};

struct ReportCatalog {
  LoadedReportConfigs loaded_reports;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_REPORT_CATALOG_H_
