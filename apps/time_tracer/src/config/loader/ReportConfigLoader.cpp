// config/loader/ReportConfigLoader.cpp
#include "ReportConfigLoader.hpp"
#include "JsonLoaderUtils.hpp"

using namespace JsonLoaderUtils; // 使用我们刚刚创建的命名空间

// ==========================================
// Daily Loaders
// ==========================================

DailyTexConfig ReportConfigLoader::loadDailyTexConfig(const fs::path& path) {
    json j = read_json(path);
    DailyTexConfig config;
    
    fill_tex_style(j, config.fonts, config.layout);
    fill_keyword_colors(j, config.keyword_colors);
    fill_daily_labels(j, config.labels);

    if (j.contains("statistics_items")) {
        parse_statistics_items(j["statistics_items"], config.statistics_items);
    }
    return config;
}

DailyTypConfig ReportConfigLoader::loadDailyTypConfig(const fs::path& path) {
    json j = read_json(path);
    DailyTypConfig config;
    
    fill_typ_style(j, config.fonts, config.layout);
    fill_keyword_colors(j, config.keyword_colors);
    fill_daily_labels(j, config.labels);

    if (j.contains("statistics_items")) {
        parse_statistics_items(j["statistics_items"], config.statistics_items);
    }
    return config;
}

DailyMdConfig ReportConfigLoader::loadDailyMdConfig(const fs::path& path) {
    json j = read_json(path);
    DailyMdConfig config;

    // MD 通常不需要复杂的字体/布局配置
    fill_daily_labels(j, config.labels);
    
    if (j.contains("statistics_items")) {
        parse_statistics_items(j["statistics_items"], config.statistics_items);
    }
    return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

MonthlyTexConfig ReportConfigLoader::loadMonthlyTexConfig(const fs::path& path) {
    json j = read_json(path);
    MonthlyTexConfig config;
    
    fill_tex_style(j, config.fonts, config.layout);
    fill_monthly_labels(j, config.labels);
    
    return config;
}

MonthlyTypConfig ReportConfigLoader::loadMonthlyTypConfig(const fs::path& path) {
    json j = read_json(path);
    MonthlyTypConfig config;
    
    fill_typ_style(j, config.fonts, config.layout);
    fill_monthly_labels(j, config.labels);
    
    return config;
}

MonthlyMdConfig ReportConfigLoader::loadMonthlyMdConfig(const fs::path& path) {
    json j = read_json(path);
    MonthlyMdConfig config;
    
    fill_monthly_labels(j, config.labels);
    
    return config;
}

// ==========================================
// Period Loaders
// ==========================================

PeriodTexConfig ReportConfigLoader::loadPeriodTexConfig(const fs::path& path) {
    json j = read_json(path);
    PeriodTexConfig config;
    
    fill_tex_style(j, config.fonts, config.layout);
    fill_period_labels(j, config.labels);
    
    return config;
}

PeriodTypConfig ReportConfigLoader::loadPeriodTypConfig(const fs::path& path) {
    json j = read_json(path);
    PeriodTypConfig config;
    
    fill_typ_style(j, config.fonts, config.layout);
    fill_period_labels(j, config.labels);
    
    return config;
}

PeriodMdConfig ReportConfigLoader::loadPeriodMdConfig(const fs::path& path) {
    json j = read_json(path);
    PeriodMdConfig config;
    
    fill_period_labels(j, config.labels);
    
    return config;
}