// config/Config.h
#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H
#include "common/ConfigTypes.hpp" // [引用] 引入数据定义
#include <string>
#include <vector>
#include <optional>
#include <toml++/toml.hpp>       // 仅 Loader 需要依赖 toml++

class ConfigLoader {
public:
    // 接收字符串内容 (pure logic)
    std::optional<TomlConfigData> load_from_content(
        const std::string& activities_content, 
        const std::string& settings_content,
        const std::string& mapping_content);

private:
    // 内部解析函数
    bool _parse_mapping_keys(const std::string& content, TomlConfigData& config_data);
    bool _parse_activities(const std::string& content, TomlConfigData& config_data);
    bool _parse_settings(const std::string& content, TomlConfigData& config_data);

    // Helper functions
    void _load_daily_remarks(const toml::table& data, TomlConfigData& config);
    void _load_activity_remarks(const toml::table& data, TomlConfigData& config);
    void _load_wake_keywords(const toml::table& data, TomlConfigData& config);
};

#endif // CONFIG_CONFIG_H