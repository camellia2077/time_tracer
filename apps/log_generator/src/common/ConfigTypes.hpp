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
    // [新增] 最大行数，默认为 1
    int max_lines = 1; 
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
    // 为了保证生成的数据能被下游 ETL 程序准确识别并入库，
    // 我们不再允许使用未定义的通用活动。
    // 唯一的活动来源必须是 mapping_config.toml 中定义的 Keys。
    // 这是从 mapping_config.toml 左侧键名解析出的活动列表，是合法的活动全集。
    std::vector<std::string> mapped_activities;
    
    std::optional<DailyRemarkConfig> remarks;
    std::optional<ActivityRemarkConfig> activity_remarks;
    std::vector<std::string> wake_keywords;
};

#endif // CONFIG_TYPES_HPP