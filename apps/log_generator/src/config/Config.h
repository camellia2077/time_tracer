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

// 新增：活动备注的数据结构
struct ActivityRemarkConfig {
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
    bool enable_nosleep = false; // 新增：是否开启通宵模式
};

struct JsonConfigData {
    std::vector<std::string> activities;
    std::optional<DailyRemarkConfig> remarks;
    std::optional<ActivityRemarkConfig> activity_remarks; // 新增
    std::vector<std::string> wake_keywords;
};


// --- 加载函数声明 ---
namespace ConfigLoader {
    std::optional<JsonConfigData> load_json_configurations(const std::string& activities_filename, const std::string& remarks_filename);
}