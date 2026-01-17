// config/validator/facade/ConfigFacade.hpp
#ifndef CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_HPP_
#define CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_HPP_

#include <string>
#include <vector>
#include <filesystem>
#include <toml++/toml.h> 

class ConfigFacade {
public:
    bool validate_converter_configs(
        const toml::table& main_tbl,
        const toml::table& mappings_tbl,
        const toml::table& duration_rules_tbl
    ) const;

    // [修改] Reports 验证现在也是 TOML
    bool validate_query_configs(
        const std::vector<std::pair<std::string, toml::table>>& query_configs
    ) const;

    bool validate_plugins(const std::filesystem::path& plugins_path) const;
};

#endif // CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_HPP_