// infra/config/models/insights_catalog.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_
#define INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_

#include <string>
#include <unordered_map>

#include "infra/config/models/insights_config_models.hpp"

struct MarkdownInsightsConfigs {
  DailyMdConfig day;
  MonthlyMdConfig month;
  PeriodMdConfig period;
  WeeklyMdConfig week;
  YearlyMdConfig year;
};

struct LoadedInsightsConfigs {
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

  MarkdownInsightsConfigs markdown;
  std::unordered_map<std::string, MarkdownInsightsConfigs> markdown_locales;
};

struct InsightsCatalog {
  LoadedInsightsConfigs loaded_insights;
  DailyStatusConfig daily_statuses;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_
