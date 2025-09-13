#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp> // forward declaration is complex, so include directly

// --- 数据结构定义 ---
struct DailyRemarkConfig {
    std::string prefix;
    std::vector<std::string> contents;
    double generation_chance = 0.5;
};

struct ActivityRemarkConfig {
    std::vector<std::string> contents;
    double generation_chance = 0.5;
};

enum class GenerationMode {
    YearRange,
    SingleYear
};

struct Config {
    int start_year;
    int end_year;
    int items_per_day;
    GenerationMode mode;
    bool enable_nosleep = false;
};

struct JsonConfigData {
    std::vector<std::string> activities;
    std::optional<DailyRemarkConfig> remarks;
    std::optional<ActivityRemarkConfig> activity_remarks;
    std::vector<std::string> wake_keywords;
};

// --- [核心修改] 重构为 ConfigLoader 类 ---
class ConfigLoader {
public:
    /**
     * @brief (公共接口) 加载所有必需的JSON配置文件。
     * @param activities_filename common_activities.json 的路径。
     * @param settings_filename activities_config.json 的路径。
     * @return 如果所有配置都成功加载，则返回包含数据的 JsonConfigData；否则返回 std::nullopt。
     */
    std::optional<JsonConfigData> load_json_configurations(const std::string& activities_filename, const std::string& settings_filename);

private:
    // --- [核心修改] 所有私有函数都以下划线开头 ---

    bool _parse_json_from_file(const std::string& filename, nlohmann::json& json_data);
    
    bool _load_activities(const std::string& filename, JsonConfigData& config_data);
    
    bool _load_settings(const std::string& filename, JsonConfigData& config_data);

    void _load_daily_remarks(const nlohmann::json& data, const std::string& filename, JsonConfigData& config);
    void _load_activity_remarks(const nlohmann::json& data, const std::string& filename, JsonConfigData& config);
    void _load_wake_keywords(const nlohmann::json& data, const std::string& filename, JsonConfigData& config);
};