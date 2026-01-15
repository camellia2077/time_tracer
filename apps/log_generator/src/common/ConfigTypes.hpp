// common/ConfigTypes.hpp
#ifndef CONFIG_TYPES_HPP
#define CONFIG_TYPES_HPP

#include <string>
#include <vector>
#include <optional>

// 生成模式枚举
enum class GenerationMode {
    YearRange,
    SingleYear
};

// 每日备注配置
struct DailyRemarkConfig {
    std::string prefix;
    std::vector<std::string> contents;
    double generation_chance = 0.5;
};

// 活动备注配置
struct ActivityRemarkConfig {
    std::vector<std::string> contents;
    double generation_chance = 0.5;
};

// 核心运行时配置
struct Config {
    int start_year;
    int end_year;
    int items_per_day;
    GenerationMode mode;
    bool enable_nosleep = false;
};

// TOML 文件对应的原始数据结构
struct TomlConfigData {
    std::vector<std::string> activities;
    std::vector<std::string> mapped_activities;
    std::optional<DailyRemarkConfig> remarks;
    std::optional<ActivityRemarkConfig> activity_remarks;
    std::vector<std::string> wake_keywords;
};

#endif // CONFIG_TYPES_HPP