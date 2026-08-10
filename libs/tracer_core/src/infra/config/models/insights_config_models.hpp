// infra/config/models/insights_config_models.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CONFIG_MODELS_H_
#define INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CONFIG_MODELS_H_

#include <map>
#include <string>
#include <vector>

// 引入拆分后的模块
#include "infra/config/models/insights_base_models.hpp"
#include "infra/config/models/insights_label_models.hpp"

// ==========================================
// 最终配置对象 (Config Objects)
// ==========================================

enum class DailyStatusType {
  kParentPresent,
};

struct DailyStatusDefinition {
  std::string id;
  std::string label;
  DailyStatusType type = DailyStatusType::kParentPresent;
  std::string parent;
};

struct DailyStatusConfig {
  int schema_version = 1;
  std::vector<DailyStatusDefinition> statuses;
};

// --- Daily Configs ---

inline constexpr int kDefaultDailyTypStatisticFontSize = 10;
inline constexpr int kDefaultDailyTypStatisticTitleFontSize = 12;

struct DailyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  DailyInsightsLabels labels;
  std::map<std::string, std::string> keyword_colors;
  std::vector<InsightsStatisticsItem> statistics_items;
};

struct DailyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  DailyInsightsLabels labels;
  std::map<std::string, std::string> keyword_colors;
  std::vector<InsightsStatisticsItem> statistics_items;
  int statistic_font_size = kDefaultDailyTypStatisticFontSize;
  int statistic_title_font_size = kDefaultDailyTypStatisticTitleFontSize;
};

struct DailyMdConfig {
  DailyInsightsLabels labels;
  std::string end_only_time_format;
};

// --- Monthly Configs ---

struct MonthlyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  MonthlyInsightsLabels labels;
};

struct MonthlyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  MonthlyInsightsLabels labels;
};

struct MonthlyMdConfig {
  MonthlyInsightsLabels labels;
};

// --- Period Configs ---

struct PeriodTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  PeriodInsightsLabels labels;
};

struct PeriodTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  PeriodInsightsLabels labels;
};

struct PeriodMdConfig {
  PeriodInsightsLabels labels;
};

// --- Weekly Configs ---

struct WeeklyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  WeeklyInsightsLabels labels;
};

struct WeeklyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  WeeklyInsightsLabels labels;
};

struct WeeklyMdConfig {
  WeeklyInsightsLabels labels;
};

// --- Yearly Configs ---

struct YearlyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  YearlyInsightsLabels labels;
};

struct YearlyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  YearlyInsightsLabels labels;
};

struct YearlyMdConfig {
  YearlyInsightsLabels labels;
};

#endif  // INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_CONFIG_MODELS_H_
