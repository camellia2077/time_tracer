// config/loader/ReportConfigLoader.cpp
#include "ReportConfigLoader.hpp"
#include "TomlLoaderUtils.hpp" // [修改] 引用新的 TOML 工具

using namespace TomlLoaderUtils; // [修改] 使用新命名空间

// ==========================================
// Daily Loaders
// ==========================================

DailyTexConfig ReportConfigLoader::loadDailyTexConfig(const fs::path& path) {
    toml::table tbl = read_toml(path); // [修改] read_json -> read_toml
    DailyTexConfig config;
    
    fill_tex_style(tbl, config.fonts, config.layout);
    fill_keyword_colors(tbl, config.keyword_colors);
    fill_daily_labels(tbl, config.labels);

    if (const toml::array* arr = tbl["statistics_items"].as_array()) {
        parse_statistics_items(arr, config.statistics_items);
    }
    return config;
}

DailyTypConfig ReportConfigLoader::loadDailyTypConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    DailyTypConfig config;
    
    fill_typ_style(tbl, config.fonts, config.layout);
    fill_keyword_colors(tbl, config.keyword_colors);
    fill_daily_labels(tbl, config.labels);

    if (const toml::array* arr = tbl["statistics_items"].as_array()) {
        parse_statistics_items(arr, config.statistics_items);
    }
    return config;
}

DailyMdConfig ReportConfigLoader::loadDailyMdConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    DailyMdConfig config;

    fill_daily_labels(tbl, config.labels);
    
    if (const toml::array* arr = tbl["statistics_items"].as_array()) {
        parse_statistics_items(arr, config.statistics_items);
    }
    return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

MonthlyTexConfig ReportConfigLoader::loadMonthlyTexConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    MonthlyTexConfig config;
    
    fill_tex_style(tbl, config.fonts, config.layout);
    fill_monthly_labels(tbl, config.labels);
    
    return config;
}

MonthlyTypConfig ReportConfigLoader::loadMonthlyTypConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    MonthlyTypConfig config;
    
    fill_typ_style(tbl, config.fonts, config.layout);
    fill_monthly_labels(tbl, config.labels);
    
    return config;
}

MonthlyMdConfig ReportConfigLoader::loadMonthlyMdConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    MonthlyMdConfig config;
    
    fill_monthly_labels(tbl, config.labels);
    
    return config;
}

// ==========================================
// Period Loaders
// ==========================================

PeriodTexConfig ReportConfigLoader::loadPeriodTexConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    PeriodTexConfig config;
    
    fill_tex_style(tbl, config.fonts, config.layout);
    fill_period_labels(tbl, config.labels);
    
    return config;
}

PeriodTypConfig ReportConfigLoader::loadPeriodTypConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    PeriodTypConfig config;
    
    fill_typ_style(tbl, config.fonts, config.layout);
    fill_period_labels(tbl, config.labels);
    
    return config;
}

PeriodMdConfig ReportConfigLoader::loadPeriodMdConfig(const fs::path& path) {
    toml::table tbl = read_toml(path);
    PeriodMdConfig config;
    
    fill_period_labels(tbl, config.labels);
    
    return config;
}