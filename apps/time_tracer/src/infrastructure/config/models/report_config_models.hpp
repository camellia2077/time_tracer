// infrastructure/config/models/models/report_config_models.hpp
#ifndef COMMON_CONFIG_MODELS_REPORT_CONFIG_MODELS_H_
#define COMMON_CONFIG_MODELS_REPORT_CONFIG_MODELS_H_

#include <map>
#include <string>
#include <vector>

// 引入拆分后的模块
#include "infrastructure/config/models/report_base_models.hpp"
#include "infrastructure/config/models/report_label_models.hpp"

// ==========================================
// 最终配置对象 (Config Objects)
// ==========================================

// --- Daily Configs ---

inline constexpr int kDefaultDailyTypStatisticFontSize = 10;
inline constexpr int kDefaultDailyTypStatisticTitleFontSize = 12;

struct DailyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  DailyReportLabels labels;
  std::map<std::string, std::string> keyword_colors;
  std::vector<ReportStatisticsItem> statistics_items;
};

struct DailyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  DailyReportLabels labels;
  std::map<std::string, std::string> keyword_colors;
  std::vector<ReportStatisticsItem> statistics_items;
  int statistic_font_size = kDefaultDailyTypStatisticFontSize;
  int statistic_title_font_size = kDefaultDailyTypStatisticTitleFontSize;
};

struct DailyMdConfig {
  DailyReportLabels labels;
  std::vector<ReportStatisticsItem> statistics_items;
};

// --- Monthly Configs ---

struct MonthlyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  MonthlyReportLabels labels;
};

struct MonthlyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  MonthlyReportLabels labels;
};

struct MonthlyMdConfig {
  MonthlyReportLabels labels;
};

// --- Period Configs ---

struct PeriodTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  PeriodReportLabels labels;
};

struct PeriodTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  PeriodReportLabels labels;
};

struct PeriodMdConfig {
  PeriodReportLabels labels;
};

// --- Weekly Configs ---

struct WeeklyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  WeeklyReportLabels labels;
};

struct WeeklyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  WeeklyReportLabels labels;
};

struct WeeklyMdConfig {
  WeeklyReportLabels labels;
};

// --- Yearly Configs ---

struct YearlyTexConfig {
  FontConfig fonts;
  LayoutConfig layout;
  YearlyReportLabels labels;
};

struct YearlyTypConfig {
  FontConfig fonts;
  LayoutConfig layout;
  YearlyReportLabels labels;
};

struct YearlyMdConfig {
  YearlyReportLabels labels;
};

#endif  // COMMON_CONFIG_MODELS_REPORT_CONFIG_MODELS_H_
