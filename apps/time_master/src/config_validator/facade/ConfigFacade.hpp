// config_validator/facade/ConfigFacade.hpp
#ifndef CONFIG_FACADE_HPP
#define CONFIG_FACADE_HPP

#include <string>
#include <vector>
#include <filesystem> // [新增] 包含 C++17 filesystem 头文件
#include <nlohmann/json.hpp>

/**
 * @class ConfigFacade
 * @brief (Top-level Facade) 协调所有子系统验证器完成配置验证。
 */
class ConfigFacade {
public:
    /**
     * @brief 验证预处理相关的配置文件。
     */
    bool validate_preprocessing_configs(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    ) const;

    /**
     * @brief 验证所有查询相关的配置文件。
     */
    bool validate_query_configs(
        const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
    ) const;

    /**
     * @brief [新增] 验证所有必需的插件是否存在。
     * @param plugins_path 指向 'plugins' 目录的路径。
     * @return 如果验证成功则返回 true，否则返回 false。
     */
    bool validate_plugins(const std::filesystem::path& plugins_path) const;
};

#endif // CONFIG_FACADE_HPP