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

struct Config {
    int start_year;
    int end_year;
    int items_per_day;
};

struct JsonConfigData {
    std::vector<std::string> activities;
    std::optional<DailyRemarkConfig> remarks;
};


// --- 加载函数声明 ---
namespace ConfigLoader {
    std::optional<JsonConfigData> load_json_configurations(const std::string& json_filename);
    std::optional<Config> parse_arguments(int argc, char* argv[]);
}