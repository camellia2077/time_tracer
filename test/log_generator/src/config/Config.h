#pragma once
#include <string>
#include <vector>
#include <optional>

// --- 数据结构定义 ---
struct DailyRemarkConfig {
    std::string prefix;
    std::vector<std::string> contents;
    double generation_chance = 0.5;
};

// [核心修改] 添加一个枚举来表示不同的生成模式
enum class GenerationMode {
    YearRange, // 按时间段生成
    SingleYear // 按单一年份生成
};

struct Config {
    int start_year;
    int end_year;
    int items_per_day;
    // [核心修改] 添加一个字段来存储当前的生成模式
    GenerationMode mode;
};

struct JsonConfigData {
    std::vector<std::string> activities;
    std::optional<DailyRemarkConfig> remarks;
};


// --- 加载函数声明 ---
namespace ConfigLoader {
    std::optional<JsonConfigData> load_json_configurations(const std::string& json_filename);
}