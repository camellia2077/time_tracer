// common/config/report_config_models.hpp
#ifndef COMMON_CONFIG_REPORT_CONFIG_MODELS_HPP_
#define COMMON_CONFIG_REPORT_CONFIG_MODELS_HPP_

#include <string>
#include <vector>
#include <map>

// 引入拆分后的基础模型和标签模型
#include "common/config/models/report_base_models.hpp"
#include "common/config/models/report_label_models.hpp"

// ==========================================
// 最终配置对象 (Config Objects)
// ==========================================
// 这些结构体负责将“基础样式”与“业务标签”组合成
// 具体的报表配置（如“LaTeX版日报配置”）

// --- Daily Configs (日报) ---

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
    // Markdown 通常不需要复杂的字体/布局配置，只需要标签和统计项
    DailyReportLabels labels;
    std::vector<ReportStatisticsItem> statistics_items;
};

// --- Monthly Configs (月报) ---

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

// --- Period Configs (周期/自定义时间段报告) ---

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

#endif // COMMON_CONFIG_REPORT_CONFIG_MODELS_HPP_