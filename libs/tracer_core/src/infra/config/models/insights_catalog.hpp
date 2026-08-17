// infra/config/models/insights_catalog.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_
#define INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_

#include <string>
#include <unordered_map>

#include "infra/config/models/insights_config_models.hpp"

struct MarkdownInsightsConfigs {
  DailyMdConfig daily;
  MonthlyMdConfig month;
  PeriodMdConfig period;
  WeeklyMdConfig week;
  YearlyMdConfig year;
};

struct LoadedInsightsConfigs {
  struct {
    DailyTexConfig daily;
    MonthlyTexConfig month;
    PeriodTexConfig period;
    WeeklyTexConfig week;
    YearlyTexConfig year;
  } latex;

  struct {
    DailyTypConfig daily;
    MonthlyTypConfig month;
    PeriodTypConfig period;
    WeeklyTypConfig week;
    YearlyTypConfig year;
  } typst;

  MarkdownInsightsConfigs markdown;
  std::unordered_map<std::string, MarkdownInsightsConfigs> markdown_locales;
};

struct InsightsStatusConfigs {
  DailyStatusConfig day;
  DailyStatusConfig week;
  DailyStatusConfig month;
  DailyStatusConfig year;
  DailyStatusConfig recent;
  DailyStatusConfig range;
};

struct InsightsCatalog {
  LoadedInsightsConfigs loaded_insights;
  InsightsStatusConfigs statuses;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CATALOG_H_
