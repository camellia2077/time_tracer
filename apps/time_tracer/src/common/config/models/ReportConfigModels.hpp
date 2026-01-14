// common/config/models/ReportConfigModels.hpp
#ifndef REPORT_CONFIG_MODELS_HPP
#define REPORT_CONFIG_MODELS_HPP

#include <map>
#include <string>
#include <vector>

// 引入拆分后的模块
#include "common/config/models/ReportBaseModels.hpp"
#include "common/config/models/ReportLabelModels.hpp"

// ==========================================
// 最终配置对象 (Config Objects)
// ==========================================

// --- Daily Configs ---

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

#endif // REPORT_CONFIG_MODELS_HPP