// src/config/Config.hpp
#ifndef CONFIG_HPP
#define CONFIG_HPP
#include "common/ConfigTypes.hpp"
#include <string>
#include <vector>
#include <optional>
#include <toml++/toml.hpp>     

class ConfigLoader {
public:
    // [修改] 删除了 unused parameter: activities_content
    std::optional<TomlConfigData> load_from_content(
        const std::string& settings_content,
        const std::string& mapping_content);

private:
    // 内部解析函数
    bool _parse_mapping_keys(const std::string& content, TomlConfigData& config_data);
    bool _parse_settings(const std::string& content, TomlConfigData& config_data);

    // Helper functions
    void _load_daily_remarks(const toml::table& data, TomlConfigData& config);
    void _load_activity_remarks(const toml::table& data, TomlConfigData& config);
    void _load_wake_keywords(const toml::table& data, TomlConfigData& config);
};

#endif // CONFIG_HPP